#include "CMakeParser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

CMakeParser::CMakeParser(const std::string& cmakeListsPath) : m_cmakeListsPath(cmakeListsPath) {}

bool CMakeParser::parse() {
    std::ifstream file(m_cmakeListsPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open CMakeLists.txt" << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        parseLine(line);
    }

    return true;
}

const std::vector<std::string>& CMakeParser::getIncludeDirectories() const { return m_includeDirectories; }
const std::vector<std::string>& CMakeParser::getCompilerFlags() const { return m_compilerFlags; }
const std::vector<std::string>& CMakeParser::getSourceFiles() const { return m_sourceFiles; }
const std::string& CMakeParser::getTargetName() const { return m_targetName; }
const std::map<std::string, std::string>& CMakeParser::getDefinitions() const { return m_definitions; }

void CMakeParser::parseLine(const std::string& line) {
    static const std::regex includeRegex(R"(include_directories\s*\((.*?)\))");
    static const std::regex flagsRegex(R"(add_compile_options\s*\((.*?)\))");
    static const std::regex sourceRegex(R"(add_executable\s*\((\w+)\s+(.*?)\))");
    static const std::regex defineRegex(R"(add_definitions\s*\((.*?)\))");
    static const std::regex setRegex(R"(set\s*\((\w+)\s+(.*?)\))");

    std::smatch match;
    if (std::regex_search(line, match, includeRegex)) {
        parseList(match[1], m_includeDirectories);
    } else if (std::regex_search(line, match, flagsRegex)) {
        parseList(match[1], m_compilerFlags);
    } else if (std::regex_search(line, match, sourceRegex)) {
        m_targetName = match[1];
        parseList(match[2], m_sourceFiles);
    } else if (std::regex_search(line, match, defineRegex)) {
        std::vector<std::string> defines;
        parseList(match[1], defines);
        for (const auto& def : defines) {
            size_t pos = def.find('=');
            if (pos != std::string::npos) {
                m_definitions[def.substr(0, pos)] = def.substr(pos + 1);
            } else {
                m_definitions[def] = "";
            }
        }
    } else if (std::regex_search(line, match, setRegex)) {
        m_definitions[match[1]] = match[2];
    }
}

void CMakeParser::parseList(const std::string& input, std::vector<std::string>& output) {
    std::istringstream iss(input);
    std::string item;
    while (iss >> item) {
        if (item.front() == '"' && item.back() == '"') {
            item = item.substr(1, item.length() - 2);
        }
        output.push_back(item);
    }
}