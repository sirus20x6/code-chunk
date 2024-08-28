#include "FunctionalInfo.h"
#include <clang-c/Index.h>
#include <iostream>
#include <fstream>
#include "llama.h"
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include "ModelLoader.h"
#include "FileUtils.h"
#include "FunctionExtractor.h"
const size_t EMBEDDING_PREVIEW_SIZE = 5;

int main(int argc, char** argv) {
std::vector<std::string> args(argv, argv + argc);
if (args.size() < 4) {
    std::cerr << "Usage: " << args[0] << " <model_path> <embedding_model_path> <source_path>\n";
    return 1;
}
const std::string modelPath = args[1];
const std::string embeddingModelPath = args[2];
const std::string sourcePath = args[3];

    llama_model* model = load_model(modelPath.c_str());
    if (!model) {
        std::cerr << "Failed to load model." << '\n';
        return 1;
    }

    llama_model* embeddingModel = load_model(embeddingModelPath.c_str());
    if (!embeddingModel) {
        std::cerr << "Failed to load embedding model." << '\n';
        llama_free_model(model);
        return 1;
    }

    try {
        std::vector<std::string> sourceLines = readSourceFiles(sourcePath);

        VisitorData data = {};
        data.model = model;
        data.sourceLines = &sourceLines;
        std::vector<FunctionInfo> functionsInfo;
        data.functionsInfo = &functionsInfo;

        CXIndex index = clang_createIndex(0, 0);
        CXTranslationUnit unit = clang_parseTranslationUnit(
            index,
            sourcePath.c_str(),
            nullptr, 0,
            nullptr, 0,
            CXTranslationUnit_None);

        if (!unit) {
            throw std::runtime_error("Unable to parse translation unit.");
        }

        CXCursor cursor = clang_getTranslationUnitCursor(unit);
        clang_visitChildren(cursor, visitor, &data);

        clang_disposeTranslationUnit(unit);
        clang_disposeIndex(index);

        for (const auto& info : functionsInfo) {
            std::cout << "Function: " << info.signature
                      << "\nStart Line: " << info.startLine
                      << "\nEnd Line: " << info.endLine
                      << "\nToken Count: " << info.tokenCount << '\n';
        }

        int totalTokens = std::accumulate(functionsInfo.begin(), functionsInfo.end(), 0,
            [](int sum, const FunctionInfo& fi) { return sum + fi.tokenCount; });
        int maxTokenCount = std::max_element(functionsInfo.begin(), functionsInfo.end(),
            [](const FunctionInfo& a, const FunctionInfo& b) { return a.tokenCount < b.tokenCount; })->tokenCount;

        int chunkSize = static_cast<int>(std::pow(2, std::ceil(std::log2(maxTokenCount))));
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

        std::cout << "Total Tokens: " << totalTokens << '\n';
        std::cout << "Minimum Chunk Size (Power of 2): " << chunkSize << '\n';
        std::cout << "Optimal Chunk Size (Power of 2): " << optimalChunkSize << '\n';
        std::cout << "Number of Chunks: " << optimalNumChunks << '\n';
        std::cout << "Min padding: " << minPadding << '\n';

        // Create a context for embedding generation
        llama_context_params cparams = llama_context_default_params();
        llama_context* ctx = llama_new_context_with_model(embeddingModel, cparams);

        std::vector<std::vector<FunctionInfo>> chunks;
        std::vector<int> chunkTokenCounts;

        // Generate embeddings for each function
        for (const auto& info : functionsInfo) {
            std::string functionText;
            for (int i = info.startLine - 1; i < info.endLine; ++i) {
                functionText += sourceLines[i] + "\n";
            }

            std::vector<llama_token> tokens = tokenize(embeddingModel, functionText);
            std::vector<float> embedding = generate_embeddings(ctx, tokens);

            // Here you would typically store or process the embedding
            // For this example, we'll just print the first few values
            std::cout << "Embedding for function " << info.signature << ": ";
            for (size_t i = 0; i < std::min(EMBEDDING_PREVIEW_SIZE, embedding.size()); ++i) {
                std::cout << embedding[i] << " ";
            }
            std::cout << "...\n";
        }

        // Clean up
        llama_free(ctx);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }

    llama_free_model(model);
    llama_free_model(embeddingModel);

    return 0;
}