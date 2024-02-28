#pragma once
#include "FunctionalInfo.h"
#include <clang-c/Index.h>
#include <vector>
#include <string>

void extractAndTokenizeFunctionText(CXCursor cursor, const CXSourceRange& range, const std::vector<std::string>& sourceLines, llama_model* model, std::vector<FunctionInfo>& functionsInfo);
CXChildVisitResult visitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
