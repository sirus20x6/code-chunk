#include "FileUtils.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

std::vector<std::string> readSourceFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(std::move(line));
    }

    return lines;
}

std::vector<std::string> readSourceFiles(const std::filesystem::path& path) {
    std::vector<std::string> lines;

    auto is_source_file = [](const std::filesystem::path& p) {
        return p.extension() == ".cpp" || p.extension() == ".h" || p.extension() == ".hpp";
    };

    try {
        if (std::filesystem::is_directory(path)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                if (std::filesystem::is_regular_file(entry) && is_source_file(entry.path())) {
                    auto file_lines = readSourceFile(entry.path().string());
                    lines.insert(lines.end(), file_lines.begin(), file_lines.end());
                    lines.emplace_back(""); // Add empty line between files
                }
            }
        } else if (std::filesystem::is_regular_file(path) && is_source_file(path)) {
            lines = readSourceFile(path.string());
        } else {
            throw std::runtime_error("Invalid path: " + path.string());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
        throw;
    }

    std::cout << "Total lines read: " << lines.size() << '\n';
    return lines;
}