#pragma once

#include <clang-c/Index.h>
#include <iostream>

// Functions for working with AST (Abstract Syntax Tree)
namespace ASTUtils {
    // Print the AST recursively starting from a given cursor
    void printAST(CXCursor cursor, int depth = 0);

    // Visit children of a given cursor
    CXChildVisitResult visitASTChildren(CXCursor cursor, CXCursorVisitor visitor, CXClientData clientData);
};
