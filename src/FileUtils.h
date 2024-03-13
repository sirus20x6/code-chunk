/**
 * @file FileUtils.h
 * @brief This file contains the declarations of file utility functions.
 */

#pragma once
#include <string>
#include <vector>
#include <filesystem>

/**
 * @brief Reads a source file and returns its content as a vector of strings.
 * @param filename The name of the file to read.
 * @return A vector of strings, where each string is a line from the file.
 */
std::vector<std::string> readSourceFile(const char* filename);

/**
 * @brief Reads source files from a directory or a single file.
 * @param path The path to the directory or file to read.
 * @return A vector of strings, where each string is a line from the files.
 *
 * This function reads all .cpp files in the given directory (recursively) or the single .cpp file.
 * It returns the content of the files as a vector of strings, where each string is a line from the files.
 */
std::vector<std::string> readSourceFiles(const std::filesystem::path& path);