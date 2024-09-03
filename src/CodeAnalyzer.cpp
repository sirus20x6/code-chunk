#include "CodeAnalyzer.h"
#include "FileUtils.h"
#include "LogUtils.h"
#include <iostream>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/MCStreamer.h>
#include <llvm/CodeGen/AsmPrinter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <fstream>

CodeAnalyzer::CodeAnalyzer(const std::string& modelPath, const std::string& embeddingModelPath, const std::string& sourcePath)
    : m_sourcePath(sourcePath), m_model(nullptr), m_embeddingModel(nullptr), m_ctx(nullptr) {
    m_cmakeParser = std::make_unique<CMakeParser>();
    m_modelLoader = std::make_unique<ModelLoader>();
    m_astParser = std::make_unique<ASTParser>();

    try {
        m_model = m_modelLoader->loadModel(modelPath.c_str());
        m_embeddingModel = m_modelLoader->loadModel(embeddingModelPath.c_str());
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load models: " + std::string(e.what()));
    }

    initializeLLVM();
}

CodeAnalyzer::~CodeAnalyzer() {
    if (m_ctx) llama_free(m_ctx);
    if (m_model) llama_free_model(m_model);
    if (m_embeddingModel) llama_free_model(m_embeddingModel);
}

void CodeAnalyzer::run() {
    parseProject();
    processFunctions();
    generateLLVMIR();
    generateAssembly();
    printFunctionInfo();
    generateEmbeddings();
    calculateChunkSize();
}

void CodeAnalyzer::parseProject() {
    LogUtils::Logger::info("Parsing CMake project");
    if (!m_cmakeParser->parse(m_sourcePath)) {
        throw std::runtime_error("Failed to parse CMake project: " + m_cmakeParser->getLastError());
    }
    m_compilerCommands = m_cmakeParser->getCompilerCommands();
    if (m_compilerCommands.empty()) {
        throw std::runtime_error("No compiler commands found. Last error: " + m_cmakeParser->getLastError());
    }
}

void CodeAnalyzer::processFunctions() {
    LogUtils::Logger::info("Processing source files");
    for (const auto& cmd : m_compilerCommands) {
        LogUtils::Logger::debug("Processing file: ", cmd.file);

        std::vector<std::string> compilerFlags = filterCompilerFlags(cmd.getFlags(), cmd.file);
        
        // Check for C++ standard flag
        auto stdIt = std::find_if(compilerFlags.begin(), compilerFlags.end(),
                                  [](const std::string& flag) { return flag.starts_with("-std="); });
        if (stdIt == compilerFlags.end()) {
            compilerFlags.push_back("-std=c++20");
        }

        // Add -fsyntax-only if not present
        if (std::find(compilerFlags.begin(), compilerFlags.end(), "-fsyntax-only") == compilerFlags.end()) {
            compilerFlags.push_back("-fsyntax-only");
        }

        // Add include path if it's not present
        std::string includePath = "-I" + (std::filesystem::path(cmd.file).parent_path().parent_path() / "include").string();
        if (std::find(compilerFlags.begin(), compilerFlags.end(), includePath) == compilerFlags.end()) {
            compilerFlags.push_back(includePath);
        }

        std::vector<FunctionInfo> functionsInfo;
        std::vector<const char*> cFlags;
        for (const auto& flag : compilerFlags) {
            cFlags.push_back(flag.c_str());
        }

        LogUtils::Logger::debug("Compiler flags:");
        for (const auto& flag : compilerFlags) {
            LogUtils::Logger::debug("  ", flag);
        }

        CXErrorCode error = m_astParser->parseFile(cmd.file, cFlags, functionsInfo);
        if (error != CXError_Success) {
            LogUtils::Logger::error("Failed to parse ", cmd.file, ". Error code: ", error);
            continue;
        }

        for (auto& fi : functionsInfo) {
            fi.filePath = cmd.file;
        }

        m_allFunctionsInfo.insert(m_allFunctionsInfo.end(), functionsInfo.begin(), functionsInfo.end());
    }
    LogUtils::Logger::info("Total number of functions found: ", m_allFunctionsInfo.size());
    
    if (m_allFunctionsInfo.empty()) {
        throw std::runtime_error("No functions were found or parsed.");
    }
}

std::vector<std::string> CodeAnalyzer::filterCompilerFlags(const std::vector<std::string>& flags, const std::string& sourceFile) {
    std::vector<std::string> filteredFlags;
    bool skipNext = false;
    for (size_t i = 0; i < flags.size(); ++i) {
        const auto& flag = flags[i];
        if (skipNext) {
            skipNext = false;
            continue;
        }
        // Filter out flags that might cause issues with Clang's LibTooling
        if (flag != "-nostdinc" && flag != "-nostdinc++" && 
            !flag.starts_with("-o") && !flag.starts_with("-c") &&
            flag != sourceFile && !flag.ends_with(".o")) {
            filteredFlags.push_back(flag);
        }
        // Skip the next flag if it's an argument to -o
        if (flag == "-o") {
            skipNext = true;
        }
    }
    return filteredFlags;
}

void CodeAnalyzer::generateEmbeddings() {
    if (m_allFunctionsInfo.empty()) {
        throw std::runtime_error("No functions were found or parsed.");
    }

    llama_context_params cparams = llama_context_default_params();
    m_ctx = llama_new_context_with_model(m_embeddingModel, cparams);
    if (!m_ctx) {
        throw std::runtime_error("Failed to create llama context");
    }

    for (const auto& info : m_allFunctionsInfo) {
        std::vector<std::string> sourceLines = FileUtils::readFile(info.filePath);
        if (sourceLines.empty() || info.startLine > sourceLines.size() || info.endLine > sourceLines.size()) {
            std::cerr << "Invalid function info for " << info.name << " in file " << info.filePath << '\n';
            continue;
        }

        std::string functionText;
        for (int i = info.startLine - 1; i < info.endLine; ++i) {
            functionText += sourceLines[i] + '\n';
        }

        std::vector<llama_token> tokens = m_modelLoader->tokenize(m_embeddingModel, functionText);
        std::vector<float> embedding = m_modelLoader->generateEmbeddings(m_ctx, tokens);

        std::cout << "Generated embedding for function " << info.name << " (size: " << embedding.size() << ")\n";
    }
}

void CodeAnalyzer::calculateChunkSize() {
    int totalTokens = std::accumulate(m_allFunctionsInfo.begin(), m_allFunctionsInfo.end(), 0,
        [](int sum, const FunctionInfo& fi) { return sum + fi.tokenCount; });
    int maxTokenCount = std::max_element(m_allFunctionsInfo.begin(), m_allFunctionsInfo.end(),
        [](const FunctionInfo& a, const FunctionInfo& b) { return a.tokenCount < b.tokenCount; })->tokenCount;

    int chunkSize = static_cast<int>(std::pow(2, std::ceil(std::log2(maxTokenCount))));
    int optimalChunkSize = chunkSize;
    int minPadding = totalTokens;
    int optimalNumChunks = 0;

    for (int currentChunkSize = chunkSize; currentChunkSize <= totalTokens; currentChunkSize *= 2) {
        int numChunks = std::ceil(static_cast<double>(totalTokens) / currentChunkSize);
        int totalCapacity = numChunks * currentChunkSize;
        int currentPadding = totalCapacity - totalTokens;

        if (currentPadding < minPadding) {
            minPadding = currentPadding;
            optimalChunkSize = currentChunkSize;
            optimalNumChunks = numChunks;
            if (minPadding == 0) break;
        } else {
            break;
        }
    }

    std::cout << "Total Tokens: " << totalTokens << '\n'
              << "Minimum Chunk Size (Power of 2): " << chunkSize << '\n'
              << "Optimal Chunk Size (Power of 2): " << optimalChunkSize << '\n'
              << "Number of Chunks: " << optimalNumChunks << '\n'
              << "Min padding: " << minPadding << '\n';
}

void CodeAnalyzer::initializeLLVM() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    m_llvmContext = std::make_unique<llvm::LLVMContext>();
    m_llvmModule = std::make_unique<llvm::Module>("CodeAnalyzerModule", *m_llvmContext);
    m_irBuilder = std::make_unique<llvm::IRBuilder<>>(*m_llvmContext);
}

void CodeAnalyzer::generateLLVMIR() {
    DEBUG_PRINT("Generating LLVM IR for extracted functions");

    for (auto& functionInfo : m_allFunctionsInfo) {
        // Create a simple function for demonstration purposes
        llvm::FunctionType* funcType = llvm::FunctionType::get(m_irBuilder->getVoidTy(), false);
        llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionInfo.name, *m_llvmModule);

        // Create a basic block and add some simple IR instructions
        llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*m_llvmContext, "entry", func);
        m_irBuilder->SetInsertPoint(entryBlock);

        // Add a simple return instruction
        m_irBuilder->CreateRetVoid();

        // Store the LLVM IR for this function
        std::string irOutput;
        llvm::raw_string_ostream irStream(irOutput);
        func->print(irStream);
        functionInfo.llvmIR = irStream.str();
    }
    // Apply optimizations
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    llvm::PassBuilder PB;
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);

    MPM.run(*m_llvmModule, MAM);
}

void CodeAnalyzer::generateAssembly() {
    DEBUG_PRINT("Generating assembly for extracted functions");

    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    m_llvmModule->setTargetTriple(targetTriple);

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

    if (!target) {
        throw std::runtime_error("Failed to lookup target: " + error);
    }

    auto CPU = "generic";
    auto features = "";

    llvm::TargetOptions opt;
    auto RM = std::optional<llvm::Reloc::Model>();
    auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, RM);

    m_llvmModule->setDataLayout(targetMachine->createDataLayout());

    for (auto& functionInfo : m_allFunctionsInfo) {
        llvm::Function* func = m_llvmModule->getFunction(functionInfo.name);
        if (!func) {
            std::cerr << "Function " << functionInfo.name << " not found in module\n";
            continue;
        }

        std::error_code EC;
        llvm::SmallString<128> tempFilename;
        llvm::sys::fs::createTemporaryFile("assembly", "s", tempFilename, llvm::sys::fs::OF_None);
        llvm::raw_fd_ostream assemblyStream(tempFilename, EC, llvm::sys::fs::OF_Text);

        if (EC) {
            std::cerr << "Could not open file: " << EC.message() << "\n";
            continue;
        }

        llvm::legacy::PassManager PM;
        
        if (targetMachine->addPassesToEmitFile(PM, assemblyStream, nullptr, llvm::CodeGenFileType::AssemblyFile)) {
            std::cerr << "TargetMachine can't emit an assembly file\n";
            continue;
        }

        PM.run(*m_llvmModule);
        assemblyStream.flush();
        assemblyStream.close();

        // Read the contents of the temporary file
        std::ifstream assemblyFile(tempFilename.c_str());
        if (assemblyFile.is_open()) {
            functionInfo.assembly = std::string(
                (std::istreambuf_iterator<char>(assemblyFile)),
                std::istreambuf_iterator<char>()
            );
            assemblyFile.close();
        } else {
            std::cerr << "Unable to open temporary assembly file\n";
        }

        // Remove the temporary file
        llvm::sys::fs::remove(tempFilename);
    }
}


void CodeAnalyzer::printFunctionInfo() {
    for (const auto& functionInfo : m_allFunctionsInfo) {
        std::cout << "Function: " << functionInfo.name << "\n";
        std::cout << "Source code:\n";
        std::vector<std::string> sourceLines = FileUtils::readFile(functionInfo.filePath);
        for (int i = functionInfo.startLine - 1; i < functionInfo.endLine; ++i) {
            std::cout << sourceLines[i] << '\n';
        }
        std::cout << "\nLLVM IR:\n" << functionInfo.llvmIR << "\n";
        std::cout << "Assembly:\n" << functionInfo.assembly << "\n";
        std::cout << "----------------------------------------\n";
    }
}