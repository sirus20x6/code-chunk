// ModelLoader.h
#pragma once

#include "llama.h"
#include <string>
#include <vector>
#include <memory>

class ModelLoader {
public:
    ModelLoader();
    ~ModelLoader();

    llama_model* loadModel(const char* modelPath);
    std::vector<llama_token> tokenize(llama_model* model, const std::string& input);
    std::vector<float> generateEmbeddings(llama_context* ctx, const std::vector<llama_token>& tokens);

private:
    void batchAddSeq(llama_batch& batch, const std::vector<llama_token>& tokens, llama_seq_id seqId);
    void normalize(const std::vector<float>& vec, std::vector<float>& out);
    void batchDecode(llama_context* ctx, llama_batch& batch, std::vector<float>& output, int nSeq, int nEmbd);
};