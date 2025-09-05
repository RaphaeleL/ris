#include "ast.h"

namespace ris {

// Program
void Program::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// FuncDecl
void FuncDecl::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// VarDecl
void VarDecl::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// BlockStmt
void BlockStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// IfStmt
void IfStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// WhileStmt
void WhileStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ForStmt
void ForStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ReturnStmt
void ReturnStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ExprStmt
void ExprStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// BinaryExpr
void BinaryExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// UnaryExpr
void UnaryExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// CallExpr
void CallExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ArrayIndexExpr
void ArrayIndexExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// StructAccessExpr
void StructAccessExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// LiteralExpr
void LiteralExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// IdentifierExpr
void IdentifierExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ris
