#include "ModelLoader.h"
#include <vector>
#include <cmath>
#include <stdexcept>

void batch_add_seq(llama_batch & batch, const std::vector<llama_token> & tokens, llama_seq_id seq_id) {
    for (size_t i = 0; i < tokens.size(); i++) {
        llama_batch_add(batch, 
                        tokens[i], 
                        static_cast<llama_pos>(i), 
                        std::vector<llama_seq_id>{seq_id}, 
                        i == tokens.size() - 1);
    }
}

void normalize(const std::vector<float>& vec, std::vector<float>& out) {
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

void batch_decode(llama_context * ctx, llama_batch & batch, std::vector<float>& output, int n_seq, int n_embd) {
    llama_kv_cache_clear(ctx);

    if (llama_decode(ctx, batch) < 0) {
        throw std::runtime_error("Failed to decode");
    }

    for (int i = 0; i < batch.n_tokens; i++) {
        if (!batch.logits[i]) {
            continue;
        }

        const float * embd = llama_get_embeddings_ith(ctx, i);
        if (embd == nullptr) {
            throw std::runtime_error("Failed to get embeddings for token " + std::to_string(i));
        }

        size_t offset = batch.seq_id[i][0] * n_embd;
        std::vector<float> out(output.begin() + offset, output.begin() + offset + n_embd);
        normalize(std::vector<float>(embd, embd + n_embd), out);
        std::copy(out.begin(), out.end(), output.begin() + offset);
    }
}

llama_model* load_model(const char* model_path) {
    llama_model_params model_params = llama_model_default_params();

    llama_model* model = llama_load_model_from_file(model_path, model_params);
    if (model == nullptr) {
        throw std::runtime_error("Failed to load model from " + std::string(model_path));
    }
    return model;
}

std::vector<llama_token> tokenize(llama_model* model, const std::string& input) {
    //std::vector<llama_token> tokens(llama_n_ctx(model));
    std::vector<llama_token> tokens(llama_n_ctx_train(model));
    int n_tokens = static_cast<int>(llama_tokenize(model, input.c_str(), static_cast<int>(input.length()), 
                                tokens.data(), static_cast<int>(tokens.size()), true, false));

    if (n_tokens < 0) {
        n_tokens = -n_tokens;
    }
    tokens.resize(n_tokens);
    return tokens;
}

std::vector<float> generate_embeddings(llama_context* ctx, const std::vector<llama_token>& tokens) {
    const int n_embd = llama_n_embd(llama_get_model(ctx));
    std::vector<float> embeddings(n_embd, 0);

    llama_batch batch = llama_batch_init(static_cast<int32_t>(tokens.size()), 0, 1);
    batch_add_seq(batch, tokens, 0);

    std::vector<float> output(tokens.size() * n_embd, 0);
    batch_decode(ctx, batch, output, 1, n_embd);

    llama_batch_free(batch);
    return embeddings;
}