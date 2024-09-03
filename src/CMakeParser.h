#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

struct CompilerCommand {
    std::string directory;
    std::string command;
    std::string file;

    std::vector<std::string> getFlags() const;
};

class CMakeParser {
public:
    CMakeParser();
    ~CMakeParser();

    bool parse(const std::string& sourcePath);
    std::vector<std::string> getTargets() const;
    std::vector<std::string> getIncludeDirectories(const std::string& targetName) const;
    std::vector<std::string> getCompileDefinitions(const std::string& targetName) const;
    std::vector<std::string> getLinkedLibraries(const std::string& targetName) const;
    std::vector<CompilerCommand> getCompilerCommands() const;
    std::string getLastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};