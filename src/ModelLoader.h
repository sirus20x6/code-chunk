#pragma once
#include "../llama.cpp/llama.h"
#include <string>
#include <vector>

static void batch_add_seq(llama_batch & batch, const std::vector<int32_t> & tokens, int seq_id);
static void batch_decode(llama_context * ctx, llama_batch & batch, float * output, int n_seq, int n_embd);
static void normalize(const float * vec, float * out, int n);
llama_model* load_model(const char* model_path);
std::vector<llama_token> tokenize(llama_model* model, const std::string& input);
std::vector<float> generate_embeddings(llama_model* model, const std::string& input);