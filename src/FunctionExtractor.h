/**
 * @file FunctionExtractor.h
 * @brief This file contains the declarations of function extractor and visitor related functions.
 */

#pragma once
#include "FunctionalInfo.h"
#include <clang-c/Index.h>
#include <vector>
#include <string>
#include "common/common.h"

/**
 * @brief Extracts and tokenizes the function text using the provided cursor, range, source lines, llama model, and functions information.
 * @param cursor The Clang cursor representing the function.
 * @param range The source range of the function.
 * @param sourceLines The vector of source lines.
 * @param model The llama model.
 * @param functionsInfo The vector of FunctionInfo structures.
 *
 * This function is used to extract the function text from the given cursor and range, tokenize it, and store the information in the provided vectors.
 */
void extractAndTokenizeFunctionText(CXCursor cursor, const CXSourceRange& range, const std::vector<std::string>& sourceLines, llama_model* model, std::vector<FunctionInfo>& functionsInfo);

/**
 * @brief The visitor function that is called by the Clang library during the traversal of the AST.
 * @param cursor The current cursor.
 * @param parent The parent cursor.
 * @param client_data The client data provided by the caller.
 * @return The result of the visit.
 *
 * This function is used to process each cursor in the AST during the traversal. It can be used to perform various operations on the cursors, such as extracting function information.
 */
CXChildVisitResult visitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
