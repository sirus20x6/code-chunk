#pragma once

#include <string>
#include <vector>
#include "FunctionalInfo.h"  // Include dependency for FunctionInfo
#include <iostream>

// Functionality to extract and manipulate function-related data
namespace FunctionExtractor {
    std::vector<FunctionInfo> extractFunctions(const std::string& sourceCode);
    void processFunctionData(const std::vector<FunctionInfo>& functions);
};
