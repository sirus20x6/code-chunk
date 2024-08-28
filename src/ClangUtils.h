#pragma once

#include <clang-c/Index.h>
#include <vector>
#include <string>

namespace ClangUtils {
    CXIndex createIndex();
    CXErrorCode parseTranslationUnit(CXIndex index, const std::string& sourceFilePath, const std::vector<const char*>& args, CXTranslationUnit* unit);
    void printDiagnostics(CXTranslationUnit unit);
    void disposeClangResources(CXIndex index, CXTranslationUnit unit);
};
