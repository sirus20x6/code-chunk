#include "ModelLoader.h"
#include "llama.cpp/common/common.h" // Adjust path as needed
#include <vector>

llama_model* load_model(const char* model_path) {
    llama_backend_init();
    llama_model_params model_params = llama_model_default_params();
    model_params.vocab_only = true;

    return llama_load_model_from_file(model_path, model_params);
}

int tokenize_and_count(llama_model* model, const std::string& input) {
    llama_context_params ctx_params = llama_context_default_params();
    llama_context* ctx = llama_new_context_with_model(model, ctx_params);
    const bool add_bos = llama_should_add_bos_token(model);
    std::vector<llama_token> tokens = ::llama_tokenize(model, input.c_str(), add_bos, true);

    return tokens.size();
}
