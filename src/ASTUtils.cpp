#include "ASTUtils.h"

namespace ASTUtils {
    // Print the AST recursively starting from a given cursor
    void printAST(CXCursor cursor, int depth) {
        if (clang_Cursor_isNull(cursor) || clang_isTranslationUnit(clang_getCursorKind(cursor)))
            return;

        CXCursorKind kind = clang_getCursorKind(cursor);
        CXString kindName = clang_getCursorKindSpelling(kind);
        CXString cursorSpelling = clang_getCursorSpelling(cursor);

        for (int i = 0; i < depth; ++i) {
            std::cout << "  ";
        }
        std::cout << clang_getCString(kindName) << ": " << clang_getCString(cursorSpelling) << std::endl;

        clang_disposeString(kindName);
        clang_disposeString(cursorSpelling);

        clang_visitChildren(
            cursor,
            [](CXCursor c, CXCursor parent, CXClientData client_data) {
                int* depth = static_cast<int*>(client_data);
                printAST(c, *depth + 1);
                return CXChildVisit_Recurse;
            },
            &depth
        );
    }

    // Visit children of a given cursor
    CXChildVisitResult visitASTChildren(CXCursor cursor, CXCursorVisitor visitor, CXClientData clientData) {
        return static_cast<CXChildVisitResult>(clang_visitChildren(cursor, visitor, clientData));
    }
}
