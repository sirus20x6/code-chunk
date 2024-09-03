#include "FileUtils.h"
#include <fstream>

namespace FileUtils {
    std::vector<std::string> readFile(const std::string& filePath) {
        std::vector<std::string> lines;
        std::string line;
        std::ifstream file(filePath);
        while (getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
        return lines;
    }

    void writeFile(const std::string& filePath, const std::vector<std::string>& lines) {
        std::ofstream file(filePath);
        for (const auto& line : lines) {
            file << line << '\n';
        }
        file.close();
    }
}
