#pragma once
#include "llama.h"
#include <string>

llama_model* load_model(const char* model_path);
int tokenize_and_count(llama_model* model, const std::string& input);