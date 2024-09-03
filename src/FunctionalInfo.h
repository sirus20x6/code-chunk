#pragma once

#include <string>
#include <vector>

// Update the FunctionInfo struct in FunctionalInfo.h
struct FunctionInfo {
    std::string name;
    std::string type;
    std::vector<std::string> parameters;
    unsigned int startLine;
    unsigned int endLine;
    unsigned int tokenCount;
    std::string filePath;
    std::string llvmIR;    // New field to store LLVM IR
    std::string assembly;  // New field to store assembly code
};