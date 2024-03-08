#pragma once
#include <string>
#include <vector>
#include <filesystem>

std::vector<std::string> readSourceFile(const char* filename);
std::vector<std::string> readSourceFiles(const std::filesystem::path& path);