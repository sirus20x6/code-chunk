#pragma once
#include <string>
#include <vector>
#include <filesystem>

/**
 * @brief Reads a source file and returns its content as a vector of strings.
 * @param filename The name of the file to read.
 * @return A vector of strings, where each string is a line from the file.
 * @throws std::runtime_error if the file cannot be opened.
 */
std::vector<std::string> readSourceFile(const std::string& filename);

/**
 * @brief Reads source files from a directory or a single file.
 * @param path The path to the directory or file to read.
 * @return A vector of strings, where each string is a line from the files.
 *
 * This function reads all .cpp, .h, and .hpp files in the given directory (recursively) or the single file.
 * It returns the content of the files as a vector of strings, where each string is a line from the files.
 * @throws std::runtime_error if the path is invalid or if there's a filesystem error.
 */
std::vector<std::string> readSourceFiles(const std::filesystem::path& path);