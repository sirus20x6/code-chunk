#include "ModelLoader.h"
#include "common/common.h"
#include "llama.h"
#include <vector>

/**
 * @brief Adds a sequence of tokens to a batch.
 *
 * This function iterates over the provided tokens and adds them to the batch with the specified sequence ID.
 * It also sets the `is_end` flag to true for the last token in the sequence.
 *
 * @param batch The batch to which the tokens are added.
 * @param tokens The vector of tokens to add.
 * @param seq_id The sequence ID for the tokens.
 */
static void batch_add_seq(llama_batch & batch, const std::vector<int32_t> & tokens, int seq_id) {
    for (size_t i = 0; i < tokens.size(); i++) {
        llama_batch_add(batch, tokens[i], i, { seq_id }, i == tokens.size() - 1);
    }
}

/**
 * @brief Decodes a batch of tokens and generates embeddings.
 *
 * This function runs the model on the provided batch to generate embeddings. It also handles clearing the
 * previous kv_cache values and normalizing the embeddings.
 *
 * @param ctx The context associated with the model.
 * @param batch The batch of tokens to decode.
 * @param output The output array for the embeddings.
 * @param n_seq The number of sequences in the batch.
 * @param n_embd The number of embedding dimensions.
 */
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

/**
 * @brief Normalizes a vector and stores the result in another vector.
 *
 * This function calculates the L2 norm of the input vector and then divides each element by the norm to
 * normalize the vector. The result is stored in the output vector.
 *
 * @param vec The input vector to normalize.
 * @param out The output vector where the normalized values are stored.
 * @param n The size of the vectors.
 */
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

/**
 * @brief Loads a language model from a file.
 *
 * This function initializes the backend and loads a language model from a file using the specified model path.
 * It sets the model parameters to load only the vocabulary.
 *
 * @param model_path The path to the model file.
 * @return A pointer to the loaded model, or nullptr if the model could not be loaded.
 */
llama_model* load_model(const char* model_path) {
    llama_backend_init();
    llama_model_params model_params = llama_model_default_params();
    model_params.vocab_only = true;

    return llama_load_model_from_file(model_path, model_params);
}

/**
 * @brief Tokenizes an input string using the provided model.
 *
 * This function tokenizes an input string using the specified model. It determines whether to add a Beginning
 * of Sentence (BOS) token based on the model's configuration.
 *
 * @param model The model to use for tokenization.
 * @param input The input string to tokenize.
 * @return A vector of tokens.
 */
std::vector<llama_token> tokenize(llama_model* model, const std::string& input) {
    llama_context_params ctx_params = llama_context_default_params();
    llama_context* ctx = llama_new_context_with_model(model, ctx_params);
    const bool add_bos = llama_should_add_bos_token(model);
    std::vector<llama_token> tokens = ::llama_tokenize(model, input.c_str(), add_bos, true);

    return tokens;
}

/**
 * @brief Generates embeddings for a sequence of tokens.
 *
 * This function generates embeddings for a sequence of tokens using the provided context. It initializes
 * a batch, adds the tokens to the batch, and then decodes the batch to obtain the embeddings.
 *
 * @param ctx The context associated with the model.
 * @param tokens The vector of tokens for which to generate embeddings.
 * @return A vector of embeddings.
 */
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
