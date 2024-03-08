#include "ModelLoader.h"
#include "../llama.cpp/common/common.h"
#include "../llama.cpp/llama.h"
#include <vector>

static void batch_add_seq(llama_batch & batch, const std::vector<int32_t> & tokens, int seq_id) {
    for (size_t i = 0; i < tokens.size(); i++) {
        llama_batch_add(batch, tokens[i], i, { seq_id }, i == tokens.size() - 1);
    }
}

static void batch_decode(llama_context * ctx, llama_batch & batch, float * output, int n_seq, int n_embd) {
    // clear previous kv_cache values (irrelevant for embeddings)
    llama_kv_cache_clear(ctx);

    // run model
    fprintf(stderr, "%s: n_tokens = %d, n_seq = %d\n", __func__, batch.n_tokens, n_seq);
    if (llama_decode(ctx, batch) < 0) {
        fprintf(stderr, "%s : failed to decode\n", __func__);
    }

    // normalize on copy
    for (int i = 0; i < batch.n_tokens; i++) {
        if (!batch.logits[i]) {
            continue;
        }

        // try to get sequence embeddings - supported only when pooling_type is not NONE
        const float * embd = llama_get_embeddings_seq(ctx, batch.seq_id[i][0]);
        if (embd == NULL) {
            embd = llama_get_embeddings_ith(ctx, i);
            if (embd == NULL) {
                fprintf(stderr, "%s: failed to get embeddings for token %d\n", __func__, i);
                continue;
            }
        }

        float * out = output + batch.seq_id[i][0] * n_embd;
        normalize(embd, out, n_embd);
    }
}

static void normalize(const float * vec, float * out, int n) {
    float norm = 0;
    for (int i = 0; i < n; i++) {
        norm += vec[i] * vec[i];
    }
    norm = sqrt(norm);
    for (int i = 0; i < n; i++) {
        out[i] = vec[i] / norm;
    }
}

llama_model* load_model(const char* model_path) {
    llama_backend_init();
    llama_model_params model_params = llama_model_default_params();
    model_params.vocab_only = true;

    return llama_load_model_from_file(model_path, model_params);
}

std::vector<llama_token> tokenize(llama_model* model, const std::string& input) {
    llama_context_params ctx_params = llama_context_default_params();
    llama_context* ctx = llama_new_context_with_model(model, ctx_params);
    const bool add_bos = llama_should_add_bos_token(model);
    std::vector<llama_token> tokens = ::llama_tokenize(model, input.c_str(), add_bos, true);

    return tokens;
}

std::vector<float> generate_embeddings(llama_context* ctx, const std::vector<int32_t>& tokens) {
    // Ensure the batch size is set to accommodate the number of tokens
    const int n_seq = 1; // Assuming a single sequence for simplicity
    const int n_embd = llama_n_embd(llama_get_model(ctx)); // Get embedding dimensions from model associated with the context
    std::vector<float> embeddings(n_embd, 0); // Initialize embeddings vector

    // Initialize batch for tokens
    llama_batch batch = llama_batch_init(tokens.size(), 0, n_seq);

    // Add tokens to the batch
    batch_add_seq(batch, tokens, 0); // Using sequence ID 0

    // Allocate output array for embeddings
    std::vector<float> output(tokens.size() * n_embd, 0);

    // Generate embeddings by decoding the batch
    batch_decode(ctx, batch, output.data(), n_seq, n_embd);

    return embeddings;
}
