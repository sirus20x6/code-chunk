// ASTParser.cpp
#include "ASTUtils.h"
#include "FileUtils.h"
#include <iostream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

#define DEBUG_PRINT(x) do { std::cout << "DEBUG: " << x << '\n'; } while (0)

// Update the VisitorData struct
struct VisitorData {
    std::vector<FunctionInfo>* functionsInfo;
    const std::string* currentFile;
};

ASTParser::ASTParser() : m_index(clang_createIndex(0, 0)) {
    if (!m_index) {
        throw std::runtime_error("Failed to create Clang index");
    }
}

ASTParser::~ASTParser() {
    if (m_index) {
        clang_disposeIndex(m_index);
    }
}

CXErrorCode ASTParser::parseFile(const std::string& filePath, const std::vector<const char*>& compilerFlags, std::vector<FunctionInfo>& functionsInfo) {
    DEBUG_PRINT("Parsing file: " << filePath);
    m_currentFile = filePath;  // Set the current file being parsed

    CXTranslationUnit unit = nullptr;
    CXErrorCode error = clang_parseTranslationUnit2(
        m_index,
        filePath.c_str(),
        compilerFlags.data(), static_cast<int>(compilerFlags.size()),
        nullptr, 0,
        CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_KeepGoing,
        &unit
    );

    if (error != CXError_Success) {
        std::cerr << "Error parsing translation unit for " << filePath << ": " << error << '\n';
        switch (error) {
            case CXError_Failure:
                std::cerr << "Generic error occurred\n";
                break;
            case CXError_Crashed:
                std::cerr << "libclang crashed while parsing\n";
                break;
            case CXError_InvalidArguments:
                std::cerr << "Invalid arguments were passed to libclang\n";
                break;
            case CXError_ASTReadError:
                std::cerr << "AST deserialization error occurred\n";
                break;
            default:
                std::cerr << "Unknown error occurred\n";
        }
        return error;
    }

    if (!unit) {
        std::cerr << "Unable to parse translation unit for " << filePath << '\n';
        return CXError_Failure;
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    
    VisitorData data;
    data.functionsInfo = &functionsInfo;
    data.currentFile = &m_currentFile;
    
    clang_visitChildren(cursor, visitorCallback, &data);

    clang_disposeTranslationUnit(unit);

    DEBUG_PRINT("Parsed " << functionsInfo.size() << " functions in " << filePath);
    return CXError_Success;
}

CXChildVisitResult ASTParser::visitorCallback(CXCursor cursor, CXCursor parent, CXClientData clientData) {
    auto* data = static_cast<VisitorData*>(clientData);
    
    if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl) {
        CXSourceLocation loc = clang_getCursorLocation(cursor);
        CXFile file;
        unsigned line, column, offset;
        clang_getSpellingLocation(loc, &file, &line, &column, &offset);
        
        CXString fileName = clang_getFileName(file);
        std::string cursorFileName = clang_getCString(fileName);
        clang_disposeString(fileName);
        
        // Only process functions from the file we're explicitly parsing
        if (cursorFileName == *(data->currentFile)) {
            FunctionInfo functionInfo;
            ASTParser().extractFunctionInfo(cursor, functionInfo);
            data->functionsInfo->push_back(functionInfo);
        }
    }

    return CXChildVisit_Recurse;
}

void ASTParser::extractFunctionInfo(CXCursor cursor, FunctionInfo& functionInfo) {
    CXString name = clang_getCursorSpelling(cursor);
    functionInfo.name = clang_getCString(name);
    clang_disposeString(name);

    CXType functionType = clang_getCursorType(cursor);
    CXString typeSpelling = clang_getTypeSpelling(functionType);
    functionInfo.type = clang_getCString(typeSpelling);
    clang_disposeString(typeSpelling);

    int numArgs = clang_Cursor_getNumArguments(cursor);
    for (int i = 0; i < numArgs; ++i) {
        CXCursor arg = clang_Cursor_getArgument(cursor, i);
        CXString argSpelling = clang_getCursorSpelling(arg);
        functionInfo.parameters.push_back(clang_getCString(argSpelling));
        clang_disposeString(argSpelling);
    }

    CXSourceRange range = clang_getCursorExtent(cursor);
    CXSourceLocation startLoc = clang_getRangeStart(range);
    CXSourceLocation endLoc = clang_getRangeEnd(range);

    unsigned startLine, startColumn, endLine, endColumn;
    clang_getSpellingLocation(startLoc, nullptr, &startLine, &startColumn, nullptr);
    clang_getSpellingLocation(endLoc, nullptr, &endLine, &endColumn, nullptr);

    functionInfo.startLine = startLine;
    functionInfo.endLine = endLine;

    CXToken* tokens;
    unsigned numTokens;
    clang_tokenize(clang_Cursor_getTranslationUnit(cursor), range, &tokens, &numTokens);
    functionInfo.tokenCount = numTokens;
    clang_disposeTokens(clang_Cursor_getTranslationUnit(cursor), tokens, numTokens);

    CXFile file;
    clang_getSpellingLocation(startLoc, &file, nullptr, nullptr, nullptr);
    CXString fileName = clang_getFileName(file);
    functionInfo.filePath = clang_getCString(fileName);
    clang_disposeString(fileName);
}

std::pair<std::string, unsigned> ASTParser::extractFunctionText(const std::vector<std::string>& lines, unsigned startLine) {
    std::stringstream functionText;
    unsigned braceCount = 0;
    unsigned endLine = startLine;

    for (unsigned i = startLine - 1; i < lines.size(); ++i) {
        functionText << lines[i] << '\n';
        for (char c : lines[i]) {
            if (c == '{') ++braceCount;
            if (c == '}') --braceCount;
        }
        if (braceCount == 0 && i > startLine - 1) {
            endLine = i + 1;
            break;
        }
    }

    return {functionText.str(), endLine};
}