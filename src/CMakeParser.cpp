// CMakeParser.cpp
#include "CMakeParser.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <stdexcept>
#include <sstream>

namespace fs = std::filesystem;

#define DEBUG_PRINT(x) do { std::cout << "DEBUG: " << x << '\n'; } while (0)

std::vector<std::string> CompilerCommand::getFlags() const {
    std::vector<std::string> flags;
    std::istringstream iss(command);
    std::string token;
    bool inQuotes = false;
    std::string currentFlag;

    // Skip the compiler executable
    iss >> token;

    char c;
    while (iss.get(c)) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            if (!currentFlag.empty()) {
                flags.push_back(currentFlag);
                currentFlag.clear();
            }
        } else {
            currentFlag += c;
        }
    }

    if (!currentFlag.empty()) {
        flags.push_back(currentFlag);
    }

    // Further split flags that might be combined
    std::vector<std::string> splitFlags;
    for (const auto& flag : flags) {
        if (flag.starts_with("-W") || flag.starts_with("-std=")) {
            std::istringstream flagStream(flag);
            std::string subFlag;
            while (std::getline(flagStream, subFlag, ' ')) {
                if (!subFlag.empty()) {
                    splitFlags.push_back(subFlag);
                }
            }
        } else {
            splitFlags.push_back(flag);
        }
    }

    return splitFlags;
}

class CMakeParser::Impl {
public:
    Impl() = default;

    bool parse(const std::string& sourcePath) {
        m_sourcePath = sourcePath;
        m_buildDir = m_sourcePath / "build";

        try {
            createBuildDirectory();
            setupCMakeFileAPIQuery();
            runCMakeCommand();
            return parseReply();
        } catch (const std::exception& e) {
            m_lastError = "Exception in CMakeParser::parse: " + std::string(e.what());
            DEBUG_PRINT(m_lastError);
            return false;
        }
    }

    std::vector<std::string> getTargets() const {
        std::vector<std::string> targets;
        if (!m_codemodel.empty()) {
            try {
                for (const auto& config : m_codemodel["configurations"]) {
                    for (const auto& target : config["targets"]) {
                        targets.push_back(target["name"]);
                    }
                }
            } catch (const std::exception& e) {
                DEBUG_PRINT("Error in getTargets: " << e.what());
            }
        }
        return targets;
    }

    std::vector<std::string> getIncludeDirectories(const std::string& targetName) const {
        return getPropertyForTarget(targetName, "includes", "path");
    }

    std::vector<std::string> getCompileDefinitions(const std::string& targetName) const {
        return getPropertyForTarget(targetName, "defines");
    }

    std::vector<std::string> getLinkedLibraries(const std::string& targetName) const {
        return getPropertyForTarget(targetName, "libraries");
    }

    std::vector<CompilerCommand> getCompilerCommands() const {
    std::vector<CompilerCommand> commands;

    if (!m_compileCommands.empty()) {
        DEBUG_PRINT("Extracting compiler commands from compile_commands.json");
        for (const auto& cmd : m_compileCommands) {
            CompilerCommand command;
            try {
                command.directory = cmd.at("directory");
                command.command = cmd.at("command");
                command.file = cmd.at("file");
            } catch (const nlohmann::json::exception& e) {
                std::cerr << "Error parsing compile command: " << e.what() << '\n';
                continue;
            }
            commands.push_back(command);
        }
    } else if (!m_codemodel.empty()) {
        DEBUG_PRINT("Extracting compiler commands from CMake codemodel");
        try {
            for (const auto& config : m_codemodel.at("configurations")) {
                for (const auto& target : config.at("targets")) {
                    if (target.contains("compileGroups")) {
                        for (const auto& group : target.at("compileGroups")) {
                            if (group.contains("compileCommands")) {
                                for (const auto& cmd : group.at("compileCommands")) {
                                    CompilerCommand command;
                                    command.directory = cmd.value("directory", "");
                                    command.command = cmd.value("command", "");
                                    command.file = cmd.value("file", "");
                                    if (!command.file.empty()) {
                                        commands.push_back(command);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } catch (const nlohmann::json::exception& e) {
            std::cerr << "Error parsing codemodel: " << e.what() << '\n';
        }
    }

    if (commands.empty()) {
        DEBUG_PRINT("No compiler commands found");
    } else {
        DEBUG_PRINT("Found " << commands.size() << " compiler commands");
    }

    return commands;
}

    std::string getLastError() const {
        return m_lastError;
    }

private:
    void createBuildDirectory() {
        DEBUG_PRINT("Creating build directory: " << m_buildDir);
        fs::create_directories(m_buildDir);
    }

    void setupCMakeFileAPIQuery() {
        DEBUG_PRINT("Setting up cmake-file-api query");
        fs::path query_dir = m_buildDir / ".cmake" / "api" / "v1" / "query";
        fs::create_directories(query_dir);
        std::ofstream query_file(query_dir / "client-cmake-parser");
        query_file << R"({"requests": [{"kind": "codemodel", "version": 2}]})";
        query_file.close();
    }

    void runCMakeCommand() {
        std::string cmakeCommand = "cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S " + m_sourcePath.string() + " -B " + m_buildDir.string();
        DEBUG_PRINT("Executing CMake command: " << cmakeCommand);
        int result = std::system(cmakeCommand.c_str());
        if (result != 0) {
            throw std::runtime_error("CMake command failed with exit code: " + std::to_string(result));
        }
    }

    bool parseReply() {
        fs::path reply_dir = m_buildDir / ".cmake" / "api" / "v1" / "reply";
        DEBUG_PRINT("Searching for reply in: " << reply_dir);
        
        if (!fs::exists(reply_dir)) {
            DEBUG_PRINT("CMake API reply directory does not exist: " << reply_dir.string());
            return readCompileCommands();
        }

        fs::path index_file = findIndexFile(reply_dir);
        if (index_file.empty()) {
            DEBUG_PRINT("Failed to find index file in " << reply_dir.string());
            return readCompileCommands();
        }

        DEBUG_PRINT("Found index file: " << index_file);
        return parseCodeModelFile(index_file, reply_dir);
    }

    fs::path findIndexFile(const fs::path& reply_dir) {
        for (const auto& entry : fs::directory_iterator(reply_dir)) {
            if (entry.path().filename().string().find("index-") != std::string::npos) {
                return entry.path();
            }
        }
        return {};
    }

    bool parseCodeModelFile(const fs::path& index_file, const fs::path& reply_dir) {
        std::ifstream index_stream(index_file);
        nlohmann::json index_json;
        index_stream >> index_json;

        DEBUG_PRINT("Index file contents: " << index_json.dump(2));

        std::string codemodel_file;
        if (index_json.contains("objects") && index_json["objects"].is_array()) {
            for (const auto& obj : index_json["objects"]) {
                if (obj["kind"] == "codemodel" && obj["version"].get<int>() == 2) {
                    codemodel_file = obj["jsonFile"].get<std::string>();
                    break;
                }
            }
        }

        if (codemodel_file.empty()) {
            DEBUG_PRINT("Failed to find codemodel-v2 reference in index file");
            return readCompileCommands();
        }

        fs::path codemodel_path = reply_dir / codemodel_file;
        DEBUG_PRINT("Reading codemodel file: " << codemodel_path);
        std::ifstream codemodel_stream(codemodel_path);
        if (!codemodel_stream.is_open()) {
            m_lastError = "Failed to open codemodel file: " + codemodel_path.string();
            DEBUG_PRINT(m_lastError);
            return readCompileCommands();
        }

        try {
            codemodel_stream >> m_codemodel;
        } catch (const nlohmann::json::parse_error& e) {
            m_lastError = "Failed to parse codemodel JSON: " + std::string(e.what());
            DEBUG_PRINT(m_lastError);
            return readCompileCommands();
        }

        if (m_codemodel.empty()) {
            m_lastError = "Parsed codemodel is empty";
            DEBUG_PRINT(m_lastError);
            return readCompileCommands();
        }

        DEBUG_PRINT("Successfully parsed codemodel");
        return true;
    }

    bool readCompileCommands() {
        fs::path compile_commands_path = m_buildDir / "compile_commands.json";
        if (!fs::exists(compile_commands_path)) {
            m_lastError = "compile_commands.json not found at: " + compile_commands_path.string();
            DEBUG_PRINT(m_lastError);
            return false;
        }

        DEBUG_PRINT("Reading compile_commands.json from: " << compile_commands_path);
        std::ifstream file(compile_commands_path);
        try {
            m_compileCommands = nlohmann::json::parse(file);
            return true;
        } catch (const nlohmann::json::parse_error& e) {
            m_lastError = "Failed to parse compile_commands.json: " + std::string(e.what());
            DEBUG_PRINT(m_lastError);
            return false;
        }
    }

    std::vector<std::string> getPropertyForTarget(const std::string& targetName, const std::string& propertyName, const std::string& subPropertyName = "") const {
        std::vector<std::string> result;
        if (!m_codemodel.empty()) {
            try {
                for (const auto& config : m_codemodel["configurations"]) {
                    for (const auto& target : config["targets"]) {
                        if (target["name"] == targetName) {
                            if (target.contains("compileGroups")) {
                                for (const auto& group : target["compileGroups"]) {
                                    if (group.contains(propertyName)) {
                                        for (const auto& item : group[propertyName]) {
                                            if (subPropertyName.empty()) {
                                                result.push_back(item);
                                            } else if (item.contains(subPropertyName)) {
                                                result.push_back(item[subPropertyName]);
                                            }
                                        }
                                    }
                                }
                            }
                            return result;
                        }
                    }
                }
            } catch (const std::exception& e) {
                DEBUG_PRINT("Error in getPropertyForTarget: " << e.what());
            }
        }
        return result;
    }

    fs::path m_sourcePath;
    fs::path m_buildDir;
    nlohmann::json m_codemodel;
    nlohmann::json m_compileCommands;
    std::string m_lastError;
};

CMakeParser::CMakeParser() : pImpl(std::make_unique<Impl>()) {}
CMakeParser::~CMakeParser() = default;

bool CMakeParser::parse(const std::string& sourcePath) {
    return pImpl->parse(sourcePath);
}

std::vector<std::string> CMakeParser::getTargets() const {
    return pImpl->getTargets();
}

std::vector<std::string> CMakeParser::getIncludeDirectories(const std::string& targetName) const {
    return pImpl->getIncludeDirectories(targetName);
}

std::vector<std::string> CMakeParser::getCompileDefinitions(const std::string& targetName) const {
    return pImpl->getCompileDefinitions(targetName);
}

std::vector<std::string> CMakeParser::getLinkedLibraries(const std::string& targetName) const {
    return pImpl->getLinkedLibraries(targetName);
}

std::vector<CompilerCommand> CMakeParser::getCompilerCommands() const {
    return pImpl->getCompilerCommands();
}

std::string CMakeParser::getLastError() const {
    return pImpl->getLastError();
}