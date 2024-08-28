#pragma once
#include "FunctionalInfo.h"
#include <clang-c/Index.h>
#include <vector>
#include <string>
#include "llama.h"
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
 * @param model The llama model used for tokenization.
 * @param functionsInfo The vector to store the function information.
 */
void extractAndTokenizeFunctionText(CXCursor cursor, const CXSourceRange& range, 
                                    const std::vector<std::string>& sourceLines, 
                                    llama_model* model, std::vector<FunctionInfo>& functionsInfo);

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
CXChildVisitResult visitor(CXCursor cursor, CXCursor parent, CXClientData client_data);