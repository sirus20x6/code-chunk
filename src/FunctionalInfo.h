// FunctionInfo.h
#pragma once
#include "../llama.cpp/llama.h"
#include <string>
#include <vector>

struct FunctionInfo {
    std::string signature;
    int startLine;
    int endLine;
    int tokenCount;
};

struct VisitorData {
    llama_model *model;
    std::vector<std::string> *sourceLines;
    std::vector<FunctionInfo> *functionsInfo;
};
