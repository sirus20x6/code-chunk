#pragma once
#include <string>
#include <vector>
#include "llama.h"

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
    llama_model* model;
    const std::vector<std::string>* sourceLines;
    std::vector<FunctionInfo>* functionsInfo;
};