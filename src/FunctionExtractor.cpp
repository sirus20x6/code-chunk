#include "FunctionExtractor.h"
#include "ModelLoader.h"
#include "common/common.h"

/**
 * @brief Extracts and tokenizes the text of a function.
 *
 * This function extracts the text of a function from the source code based on the provided range.
 * It then tokenizes the function text using the provided model and stores the function information
 * in the provided vector.
 *
 * @param cursor The cursor representing the function.
 * @param range The source range of the function.
 * @param sourceLines The vector containing the source code lines.
 * @param model The model used for tokenization.
 * @param functionsInfo The vector to store the function information.
 */
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

    int tokenCount = tokenize(model, functionText).size();
    CXString cursorSpelling = clang_getCursorSpelling(cursor);
    std::string functionSignature = clang_getCString(cursorSpelling);
    functionsInfo.push_back({functionSignature, static_cast<int>(startLine), static_cast<int>(endLine), tokenCount});

    clang_disposeString(cursorSpelling);
}

/**
 * @brief The visitor function called by the Clang AST traversal.
 *
 * This function is called for each cursor in the AST traversal. It checks if the cursor is from the
 * main file and if it represents a function or method declaration. If so, it extracts and tokenizes
 * the function text.
 *
 * @param cursor The current cursor being visited.
 * @param parent The parent cursor of the current cursor.
 * @param client_data Pointer to the client data, which should be a VisitorData pointer.
 * @return CXChildVisitResult The result of the visit, indicating whether to continue or recurse.
 */
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
