/**
 * @file ModelLoader.h
 * @brief This header file declares functions for loading and tokenizing models.
 *
 * The ModelLoader module provides functionality to load a language model from a file and tokenize
 * input strings using the loaded model. It also includes functions to generate embeddings for
 * tokenized sequences and to normalize vectors.
 *
 * The following functions are declared in this header:
 * - load_model: Loads a language model from a file.
 * - tokenize: Tokenizes an input string using the provided model.
 * - generate_embeddings: Generates embeddings for a sequence of tokens.
 * - batch_add_seq: Adds a sequence of tokens to a batch.
 * - batch_decode: Decodes a batch of tokens and generates embeddings.
 * - normalize: Normalizes a vector and stores the result in another vector.
 *
 * These functions are used in the context of a language model and are designed to be used by other
 * modules in the program.
 */

#pragma once
#include "llama.h"
#include <string>
#include <vector>

static void batch_add_seq(llama_batch & batch, const std::vector<int32_t> & tokens, int seq_id);
static void batch_decode(llama_context * ctx, llama_batch & batch, float * output, int n_seq, int n_embd);
static void normalize(const float * vec, float * out, int n);
llama_model* load_model(const char* model_path);
std::vector<llama_token> tokenize(llama_model* model, const std::string& input);
std::vector<float> generate_embeddings(llama_model* model, const std::string& input);