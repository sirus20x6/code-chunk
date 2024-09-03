// ModelLoader.cpp
#include "ModelLoader.h"
#include <iostream>
#include <stdexcept>
#include <cmath>

#define DEBUG_PRINT(x) do { std::cout << "DEBUG: " << x << '\n'; } while (0)

ModelLoader::ModelLoader() = default;

ModelLoader::~ModelLoader() = default;

llama_model* ModelLoader::loadModel(const char* modelPath) {
    llama_model_params modelParams = llama_model_default_params();
    llama_model* model = llama_load_model_from_file(modelPath, modelParams);
    
    if (model == nullptr) {
        throw std::runtime_error("Failed to load model from " + std::string(modelPath));
    }
    
    DEBUG_PRINT("Model loaded successfully from " << modelPath);
    return model;
}

std::vector<llama_token> ModelLoader::tokenize(llama_model* model, const std::string& input) {
    std::vector<llama_token> tokens(llama_n_ctx_train(model));
    int nTokens = llama_tokenize(model, input.c_str(), static_cast<int>(input.length()), 
                                 tokens.data(), static_cast<int>(tokens.size()), true, false);

    if (nTokens < 0) {
        nTokens = -nTokens;
    }
    
    tokens.resize(nTokens);
    DEBUG_PRINT("Tokenized input. Token count: " << nTokens);
    return tokens;
}

std::vector<float> ModelLoader::generateEmbeddings(llama_context* ctx, const std::vector<llama_token>& tokens) {
    const int nEmbd = llama_n_embd(llama_get_model(ctx));
    
    llama_batch batch = llama_batch_init(static_cast<int32_t>(tokens.size()), 0, 1);
    batchAddSeq(batch, tokens, 0);

    std::vector<float> output(tokens.size() * nEmbd, 0);
    batchDecode(ctx, batch, output, 1, nEmbd);

    llama_batch_free(batch);
    DEBUG_PRINT("Generated embeddings. Size: " << output.size());
    return output;
}

void ModelLoader::batchAddSeq(llama_batch& batch, const std::vector<llama_token>& tokens, llama_seq_id seqId) {
    for (size_t i = 0; i < tokens.size(); i++) {
        llama_batch_add(batch, 
                        tokens[i], 
                        static_cast<llama_pos>(i), 
                        std::vector<llama_seq_id>{seqId}, 
                        i == tokens.size() - 1);
    }
}

void ModelLoader::normalize(const std::vector<float>& vec, std::vector<float>& out) {
    float norm = 0;
    for (const auto& v : vec) {
        norm += v * v;
    }
    norm = std::sqrt(norm);
    if (norm == 0) {
        throw std::runtime_error("Cannot normalize zero vector");
    }
    for (size_t i = 0; i < vec.size(); i++) {
        out[i] = vec[i] / norm;
    }
}

void ModelLoader::batchDecode(llama_context* ctx, llama_batch& batch, std::vector<float>& output, int nSeq, int nEmbd) {
    llama_kv_cache_clear(ctx);

    if (llama_decode(ctx, batch) < 0) {
        throw std::runtime_error("Failed to decode");
    }

    for (int i = 0; i < batch.n_tokens; i++) {
        if (!batch.logits[i]) {
            continue;
        }

        const float* embd = llama_get_embeddings_ith(ctx, i);
        if (embd == nullptr) {
            throw std::runtime_error("Failed to get embeddings for token " + std::to_string(i));
        }

        size_t offset = batch.seq_id[i][0] * nEmbd;
        std::vector<float> out(output.begin() + offset, output.begin() + offset + nEmbd);
        normalize(std::vector<float>(embd, embd + nEmbd), out);
        std::copy(out.begin(), out.end(), output.begin() + offset);
    }
}