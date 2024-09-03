#pragma once

#include <string>
#include <vector>

// Declarations for utility functions to handle file operations
namespace FileUtils {
    std::vector<std::string> readFile(const std::string& filePath);
    void writeFile(const std::string& filePath, const std::vector<std::string>& lines);
};