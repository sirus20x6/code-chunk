#ifndef PCH_H
#define PCH_H

// Standard library headers
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <algorithm>

// LLVM headers
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>

// Clang headers
#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

// LLAMA.cpp headers
#include "common.h"
#include "ggml.h"
#include "llama.h"

// Project-specific headers
// Add your project's common headers here

#endif // PCH_H