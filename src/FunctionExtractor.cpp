#include "FunctionExtractor.h"
#include <regex>

namespace FunctionExtractor {
    std::vector<FunctionInfo> extractFunctions(const std::string& sourceCode) {
        std::vector<FunctionInfo> functions;
        std::regex funcRegex(R"(\\b(\\w+)\\s+(\\w+)\\s*\\(([^)]*)\\))"); // Simplified regex for demonstration
        std::smatch matches;

        std::string::const_iterator searchStart(sourceCode.cbegin());
        while (std::regex_search(searchStart, sourceCode.cend(), matches, funcRegex)) {
            FunctionInfo fi;
            fi.name = matches[2];
            fi.type = matches[1];
            // Process parameters if needed
            functions.push_back(fi);
            searchStart = matches.suffix().first;
        }

        return functions;
    }

    void processFunctionData(const std::vector<FunctionInfo>& functions) {
        for (const auto& func : functions) {
            std::cout << "Function: " << func.name << " Type: " << func.type << '\n';
        }
    }
}
