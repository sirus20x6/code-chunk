#ifndef CMAKE_PARSER_H
#define CMAKE_PARSER_H

#include <string>
#include <vector>
#include <map>

class CMakeParser {
public:
    CMakeParser(const std::string& cmakeListsPath);

    bool parse();

    const std::vector<std::string>& getIncludeDirectories() const;
    const std::vector<std::string>& getCompilerFlags() const;
    const std::vector<std::string>& getSourceFiles() const;
    const std::string& getTargetName() const;
    const std::map<std::string, std::string>& getDefinitions() const;

private:
    void parseLine(const std::string& line);
    void parseList(const std::string& input, std::vector<std::string>& output);

    std::string m_cmakeListsPath;
    std::vector<std::string> m_includeDirectories;
    std::vector<std::string> m_compilerFlags;
    std::vector<std::string> m_sourceFiles;
    std::string m_targetName;
    std::map<std::string, std::string> m_definitions;
};

#endif // CMAKE_PARSER_H