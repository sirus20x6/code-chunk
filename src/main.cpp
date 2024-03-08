#include "FunctionalInfo.h"
#include <clang-c/Index.h>
#include <iostream>
#include <fstream>
#include "../llama.cpp/common/common.h"
#include "../llama.cpp/llama.h"
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include "ModelLoader.h"
#include "FileUtils.h"
#include "FunctionExtractor.h"


int main(int argc, char** argv) {
    // Adjusting the argument count check to expect one more argument
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <model_path> <model_type> <filename>\n";
        return 1;
    }

    // Additional setup or configuration based on the model_type could be done here
    const std::string modelPath = argv[1];
    const std::string modelType = argv[2]; // New: Use the model type argument
    const std::string filename = argv[3];

    llama_model* model = load_model(argv[1]);
    if (!model) {
        std::cerr << "Failed to load model." << std::endl;
        return 1;
    }

    llama_model* embeddingModel = load_model(argv[2]);
    if (!embeddingModel) {
        std::cerr << "Failed to load embedding model." << std::endl;
        return 1;
    }

    std::vector<std::string> sourceLines = readSourceFiles(argv[2]);

    VisitorData data;
    data.model = model;
    data.sourceLines = &sourceLines;
    std::vector<FunctionInfo> functionsInfo;
    data.functionsInfo = &functionsInfo;

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        argv[2],
        nullptr, 0,
        nullptr, 0,
        CXTranslationUnit_None);

    if (unit == nullptr) {
        std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
        return 1;
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(cursor, visitor, &data);

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);

    for (const auto& info : functionsInfo) {
        std::cout << "Function: " << info.signature
                  << "\nStart Line: " << info.startLine
                  << "\nEnd Line: " << info.endLine
                  << "\nToken Count: " << info.tokenCount << std::endl;
    }

    int totalTokens = 0;
    int maxTokenCount = 0;
    for (const auto& info : functionsInfo) {
        totalTokens += info.tokenCount;
        maxTokenCount = std::max(maxTokenCount, info.tokenCount);
    }

    int chunkSize = std::pow(2, std::ceil(std::log2(maxTokenCount)));
    int optimalChunkSize = chunkSize;
    int minPadding = totalTokens;
    int optimalNumChunks = 0;
    for (int currentChunkSize = chunkSize; currentChunkSize <= totalTokens; currentChunkSize *= 2) {
        int numChunks = std::ceil(static_cast<double>(totalTokens) / currentChunkSize);
        int totalCapacity = numChunks * currentChunkSize;
        int currentPadding = totalCapacity - totalTokens;

        if (currentPadding < minPadding) {
            minPadding = currentPadding;
            optimalChunkSize = currentChunkSize;
            optimalNumChunks = numChunks;

            if (minPadding == 0) break;
        } else {
            break;
        }
    }

    std::cout << "Total Tokens: " << totalTokens << std::endl;
    std::cout << "Minimum Chunk Size (Power of 2): " << chunkSize << std::endl;
    std::cout << "Optimal Chunk Size (Power of 2): " << optimalChunkSize << std::endl;
    std::cout << "Number of Chunks: " << optimalNumChunks << std::endl;
    std::cout << "Min padding: " << minPadding << std::endl;

    std::vector<std::vector<FunctionInfo>> chunks;
    std::vector<int> chunkTokenCounts;

    for (const auto& func : functionsInfo) {
        bool addedToChunk = false;

        for (size_t i = 0; i < chunks.size(); ++i) {
            if (chunkTokenCounts[i] + func.tokenCount <= optimalChunkSize) {
                chunks[i].push_back(func);
                chunkTokenCounts[i] += func.tokenCount;
                addedToChunk = true;
                break;
            }
        }

        if (!addedToChunk) {
            std::vector<FunctionInfo> newChunk = {func};
            chunks.push_back(newChunk);
            chunkTokenCounts.push_back(func.tokenCount);
        }
    }

    for (size_t i = 0; i < chunks.size(); ++i) {
        int chunkTokenCount = std::accumulate(chunks[i].begin(), chunks[i].end(), 0,
            [](int sum, const FunctionInfo& fi) { return sum + fi.tokenCount; });

        std::cout << "Chunk " << i + 1 << " (Total Tokens: " << chunkTokenCount << "):" << std::endl;
        for (const auto& func : chunks[i]) {
            std::cout << "  - " << func.signature << " (" << func.tokenCount << " tokens)" << std::endl;
        }
    }

    llama_free_model(model); // Free the main model
    llama_free_model(embeddingModel); // Free the embedding model

    return 0;
}