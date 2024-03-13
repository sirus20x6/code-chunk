/**
 * @file FunctionInfo.h
 * @brief This file contains the declarations of the FunctionInfo struct and the VisitorData struct.
 */

#pragma once
#include "llama.h"
#include <string>
#include <vector>

/**
 * @brief Structure to hold information about a function.
 */
struct FunctionInfo {
    std::string signature;
    int startLine;
    int endLine;
    int tokenCount;
};

/**
 * @brief Structure to hold data used by the visitor during the parsing process.
 */
struct VisitorData {
    llama_model *model;
    std::vector<std::string> *sourceLines;
    std::vector<FunctionInfo> *functionsInfo;
};
