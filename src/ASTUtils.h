// ASTUtils.h
#pragma once

#include "FunctionalInfo.h"
#include <string>
#include <vector>
#include <clang-c/Index.h>

class ASTParser {
public:
    ASTParser();
    ~ASTParser();

    CXErrorCode parseFile(const std::string& filePath, const std::vector<const char*>& compilerFlags, std::vector<FunctionInfo>& functionsInfo);

private:
    static CXChildVisitResult visitorCallback(CXCursor cursor, CXCursor parent, CXClientData clientData);
    void extractFunctionInfo(CXCursor cursor, FunctionInfo& functionInfo);
    std::pair<std::string, unsigned> extractFunctionText(const std::vector<std::string>& lines, unsigned startLine);
    std::string m_currentFile;

    CXIndex m_index;
};