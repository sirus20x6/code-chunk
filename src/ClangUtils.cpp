#include "ClangUtils.h"
#include <iostream>

namespace ClangUtils {

CXIndex createIndex() {
    return clang_createIndex(0, 0);  // Adjust parameters as needed for your application
}

CXErrorCode parseTranslationUnit(CXIndex index, const std::string& sourceFilePath, const std::vector<const char*>& args, CXTranslationUnit* unit) {
    return clang_parseTranslationUnit2(
        index,
        sourceFilePath.c_str(),
        args.data(),
        static_cast<int>(args.size()),
        nullptr,
        0,
        CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_KeepGoing | CXTranslationUnit_SkipFunctionBodies,
        unit
    );
}

void printDiagnostics(CXTranslationUnit unit) {
    if (!unit) return;
    unsigned numDiags = clang_getNumDiagnostics(unit);
    for (unsigned i = 0; i < numDiags; ++i) {
        CXDiagnostic diag = clang_getDiagnostic(unit, i);
        CXString diagString = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());
        std::cerr << "Diagnostic: " << clang_getCString(diagString) << '\n';
        clang_disposeString(diagString);
        clang_disposeDiagnostic(diag);
    }
}

void disposeClangResources(CXIndex index, CXTranslationUnit unit) {
    if (unit) clang_disposeTranslationUnit(unit);
    if (index) clang_disposeIndex(index);
}

} // namespace ClangUtils