/**
 * @file FileUtil.cpp
 * @brief This file contains the implementation of file utility functions.
 */

#include "FileUtils.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

/**
 * @brief Reads a source file and returns its content as a vector of strings.
 * @param filename The name of the file to read.
 * @return A vector of strings, where each string is a line from the file.
 */
std::vector<std::string> readSourceFile(const char* filename) {
    std::ifstream file(filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    return lines;
}

namespace fs = std::filesystem;

/**
 * @brief Reads source files from a directory or a single file.
 * @param path The path to the directory or file to read.
 * @return A vector of strings, where each string is a line from the files.
 *
 * This function reads all .cpp files in the given directory (recursively) or the single .cpp file.
 * It returns the content of the files as a vector of strings, where each string is a line from the files.
 */
std::vector<std::string> readSourceFiles(const fs::path& path) {
    std::vector<std::string> lines;

    auto is_cpp_file = [](const fs::path& path) {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return ext == ".cpp";
    };

    if (fs::is_directory(path)) {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file() && is_cpp_file(entry.path())) {
                std::ifstream file(entry.path());
                if (!file) {
                    std::cerr << "Failed to open file: " << entry.path() << std::endl;
                    continue;
                }
                std::string line;
                while (std::getline(file, line)) {
                    lines.push_back(line);
                }
                if (!line.empty()) {
                    lines.push_back("");
                }
            }
        }
    } else if (fs::is_regular_file(path) && is_cpp_file(path)) {
        std::ifstream file(path);
        if (!file) {
            std::cerr << "Failed to open file: " << path << std::endl;
            return lines;
        }
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        if (!line.empty()) {
            lines.push_back("");
        }
    }
    std::cout << lines.size();
    return lines;
}
