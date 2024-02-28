#include "FunctionExtractor.h"
#include "ModelLoader.h"

void extractAndTokenizeFunctionText(CXCursor cursor, const CXSourceRange& range, const std::vector<std::string>& sourceLines, llama_model* model, std::vector<FunctionInfo>& functionsInfo){
    CXSourceLocation startLoc = clang_getRangeStart(range);
    CXSourceLocation endLoc = clang_getRangeEnd(range);
    unsigned startLine, startColumn, endLine, endColumn;
    clang_getSpellingLocation(startLoc, nullptr, &startLine, &startColumn, nullptr);
    clang_getSpellingLocation(endLoc, nullptr, &endLine, &endColumn, nullptr);

    std::string functionText;
    for (unsigned i = startLine; i <= endLine; ++i) {
        functionText += (i == startLine ? sourceLines[i - 1].substr(startColumn - 1) : sourceLines[i - 1]) + "\n";
    }

    int tokenCount = tokenize_and_count(model, functionText);
    CXString cursorSpelling = clang_getCursorSpelling(cursor);
    std::string functionSignature = clang_getCString(cursorSpelling);
    functionsInfo.push_back({functionSignature, static_cast<int>(startLine), static_cast<int>(endLine), tokenCount});

    clang_disposeString(cursorSpelling);
}

CXChildVisitResult visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    VisitorData* data = reinterpret_cast<VisitorData*>(client_data);
    if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
        return CXChildVisit_Continue;

    CXSourceRange range = clang_getCursorExtent(cursor);
    switch (cursor.kind) {
        case CXCursor_FunctionDecl:
        case CXCursor_CXXMethod: {
            extractAndTokenizeFunctionText(cursor, range, *(data->sourceLines), data->model, *(data->functionsInfo));
            break;
        }
        default:
            break;
    }

    return CXChildVisit_Recurse;
}
