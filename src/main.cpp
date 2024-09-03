#include "CodeAnalyzer.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <model_path> <embedding_model_path> <source_path>\n";
        return 1;
    }

    try {
        CodeAnalyzer analyzer(argv[1], argv[2], argv[3]);
        analyzer.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}