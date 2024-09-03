#pragma once

#include "CMakeParser.h"
#include "ModelLoader.h"
#include "ASTUtils.h"
#include <string>
#include <vector>
#include <memory>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

class CodeAnalyzer {
public:
    CodeAnalyzer(const std::string& modelPath, const std::string& embeddingModelPath, const std::string& sourcePath);
    ~CodeAnalyzer();

    void run();

private:
    void parseProject();
    void processFunctions();
    void generateEmbeddings();
    void calculateChunkSize();
    std::vector<std::string> filterCompilerFlags(const std::vector<std::string>& flags, const std::string& sourceFile);
    void generateLLVMIR();
    void generateAssembly();
    void initializeLLVM();
    void printFunctionInfo();  // New method to print function information

    std::unique_ptr<CMakeParser> m_cmakeParser;
    std::unique_ptr<ModelLoader> m_modelLoader;
    std::unique_ptr<ASTParser> m_astParser;

    std::string m_sourcePath;
    std::vector<CompilerCommand> m_compilerCommands;
    std::vector<FunctionInfo> m_allFunctionsInfo;
    
    llama_model* m_model;
    llama_model* m_embeddingModel;
    llama_context* m_ctx;

    std::unique_ptr<llvm::LLVMContext> m_llvmContext;
    std::unique_ptr<llvm::Module> m_llvmModule;
    std::unique_ptr<llvm::IRBuilder<>> m_irBuilder;
};