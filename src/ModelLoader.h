#pragma once
#include "common.h"
#include "ggml.h"
#include "llama.h"
#include <string>
#include <vector>

void batch_add_seq(llama_batch & batch, const std::vector<llama_token> & tokens, llama_seq_id seq_id);
void normalize(const float * vec, float * out, int n);
void batch_decode(llama_context * ctx, llama_batch & batch, float * output, int n_seq, int n_embd);
void batch_decode(llama_context * ctx, llama_batch & batch, std::vector<float>& output, int n_seq, int n_embd) ;
llama_model* load_model(const char* model_path);
std::vector<llama_token> tokenize(llama_model* model, const std::string& input);
std::vector<float> generate_embeddings(llama_context* ctx, const std::vector<llama_token>& tokens);
std::vector<float> generate_embeddings(llama_context* ctx, const std::vector<llama_token>& tokens) ;