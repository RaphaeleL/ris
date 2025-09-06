#pragma once

#include "token.h"
#include <string>
#include <vector>
#include <memory>

namespace ris {

// Forward declarations
class ASTNode;
class Program;
class FuncDecl;
class VarDecl;
class BlockStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class SwitchStmt;
class CaseStmt;
class BreakStmt;
class ContinueStmt;
class ReturnStmt;
class ExprStmt;
class BinaryExpr;
class UnaryExpr;
class CallExpr;
class StructAccessExpr;
class LiteralExpr;
class IdentifierExpr;
class ListLiteralExpr;
class ListIndexExpr;
class ListMethodCallExpr;
class PreIncrementExpr;
class PostIncrementExpr;

// Base AST node class
class ASTNode {
public:
    virtual ~ASTNode() = default;
    SourcePos position;
    
    ASTNode() = default;
    explicit ASTNode(const SourcePos& pos) : position(pos) {}
    
    // Virtual methods for visitor pattern (for future use)
    virtual void accept(class ASTVisitor& visitor) = 0;
};

// Expression base class
class Expr : public ASTNode {
public:
    Expr() = default;
    explicit Expr(const SourcePos& pos) : ASTNode(pos) {}
};

// Statement base class
class Stmt : public ASTNode {
public:
    Stmt() = default;
    explicit Stmt(const SourcePos& pos) : ASTNode(pos) {}
};

// Program node
class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<FuncDecl>> functions;
    std::vector<std::unique_ptr<VarDecl>> globals;
    
    void accept(class ASTVisitor& visitor) override;
};

// Function declaration
class FuncDecl : public ASTNode {
public:
    std::string name;
    std::string return_type;
    std::vector<std::pair<std::string, std::string>> parameters; // (type, name) pairs
    std::unique_ptr<BlockStmt> body;
    
    FuncDecl(const std::string& n, const std::string& ret_type, const SourcePos& pos)
        : ASTNode(pos), name(n), return_type(ret_type) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Variable declaration
class VarDecl : public Stmt {
public:
    std::string name;
    std::string type;
    std::unique_ptr<Expr> initializer;
    
    VarDecl(const std::string& n, const std::string& t, const SourcePos& pos)
        : Stmt(pos), name(n), type(t) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Block statement
class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    
    BlockStmt(const SourcePos& pos) : Stmt(pos) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// If statement
class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::unique_ptr<Stmt> else_branch; // nullptr if no else
    
    IfStmt(std::unique_ptr<Expr> cond, const SourcePos& pos)
        : Stmt(pos), condition(std::move(cond)) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// While statement
class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    
    WhileStmt(std::unique_ptr<Expr> cond, const SourcePos& pos)
        : Stmt(pos), condition(std::move(cond)) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// For statement
class ForStmt : public Stmt {
public:
    std::unique_ptr<VarDecl> init; // nullptr if no initialization
    std::unique_ptr<Expr> condition; // nullptr if no condition
    std::unique_ptr<Expr> update; // nullptr if no update
    std::unique_ptr<Stmt> body;
    
    ForStmt(const SourcePos& pos) : Stmt(pos) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Return statement
class ReturnStmt : public Stmt {
public:
    std::unique_ptr<Expr> value; // nullptr for void return
    
    ReturnStmt(const SourcePos& pos) : Stmt(pos) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Break statement
class BreakStmt : public Stmt {
public:
    BreakStmt(const SourcePos& pos) : Stmt(pos) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Continue statement
class ContinueStmt : public Stmt {
public:
    ContinueStmt(const SourcePos& pos) : Stmt(pos) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Case statement
class CaseStmt : public Stmt {
public:
    std::unique_ptr<Expr> value; // nullptr for default case
    std::vector<std::unique_ptr<Stmt>> statements;
    
    CaseStmt(const SourcePos& pos) : Stmt(pos) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Switch statement
class SwitchStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    std::vector<std::unique_ptr<CaseStmt>> cases;
    
    SwitchStmt(std::unique_ptr<Expr> expr, const SourcePos& pos)
        : Stmt(pos), expression(std::move(expr)) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Expression statement
class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    
    ExprStmt(std::unique_ptr<Expr> expr, const SourcePos& pos)
        : Stmt(pos), expression(std::move(expr)) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Binary expression
class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    TokenType op;
    
    BinaryExpr(std::unique_ptr<Expr> l, std::unique_ptr<Expr> r, TokenType o, const SourcePos& pos)
        : Expr(pos), left(std::move(l)), right(std::move(r)), op(o) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Unary expression
class UnaryExpr : public Expr {
public:
    std::unique_ptr<Expr> operand;
    TokenType op;
    
    UnaryExpr(std::unique_ptr<Expr> opd, TokenType o, const SourcePos& pos)
        : Expr(pos), operand(std::move(opd)), op(o) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Function call expression
class CallExpr : public Expr {
public:
    std::string function_name;
    std::vector<std::unique_ptr<Expr>> arguments;
    
    CallExpr(const std::string& name, const SourcePos& pos)
        : Expr(pos), function_name(name) {}
    
    void accept(class ASTVisitor& visitor) override;
};


// Struct access expression
class StructAccessExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::string field_name;
    
    StructAccessExpr(std::unique_ptr<Expr> obj, const std::string& field, const SourcePos& pos)
        : Expr(pos), object(std::move(obj)), field_name(field) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Literal expression
class LiteralExpr : public Expr {
public:
    std::string value;
    TokenType type; // INTEGER_LITERAL, FLOAT_LITERAL, etc.
    
    LiteralExpr(const std::string& val, TokenType t, const SourcePos& pos)
        : Expr(pos), value(val), type(t) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// Identifier expression
class IdentifierExpr : public Expr {
public:
    std::string name;
    
    IdentifierExpr(const std::string& n, const SourcePos& pos)
        : Expr(pos), name(n) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// List literal expression [1, 2, 3]
class ListLiteralExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;
    
    ListLiteralExpr(const SourcePos& pos) : Expr(pos) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// List index expression list[2]
class ListIndexExpr : public Expr {
public:
    std::unique_ptr<Expr> list;
    std::unique_ptr<Expr> index;
    
    ListIndexExpr(std::unique_ptr<Expr> l, std::unique_ptr<Expr> i, const SourcePos& pos)
        : Expr(pos), list(std::move(l)), index(std::move(i)) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// List method call expression list.push(5), list.pop(), list.size()
class ListMethodCallExpr : public Expr {
public:
    std::unique_ptr<Expr> list;
    std::string method_name; // "push", "pop", "size"
    std::vector<std::unique_ptr<Expr>> arguments; // for push method
    
    ListMethodCallExpr(std::unique_ptr<Expr> l, const std::string& method, 
                      std::vector<std::unique_ptr<Expr>> args, const SourcePos& pos)
        : Expr(pos), list(std::move(l)), method_name(method), arguments(std::move(args)) {}
    
    void accept(class ASTVisitor& visitor) override;
};

class PreIncrementExpr : public Expr {
public:
    std::unique_ptr<Expr> operand;
    
    PreIncrementExpr(std::unique_ptr<Expr> opd, const SourcePos& pos)
        : Expr(pos), operand(std::move(opd)) {}
    
    void accept(class ASTVisitor& visitor) override;
};

class PostIncrementExpr : public Expr {
public:
    std::unique_ptr<Expr> operand;
    
    PostIncrementExpr(std::unique_ptr<Expr> opd, const SourcePos& pos)
        : Expr(pos), operand(std::move(opd)) {}
    
    void accept(class ASTVisitor& visitor) override;
};

// AST Visitor interface (for future use)
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(Program& node) = 0;
    virtual void visit(FuncDecl& node) = 0;
    virtual void visit(VarDecl& node) = 0;
    virtual void visit(BlockStmt& node) = 0;
    virtual void visit(IfStmt& node) = 0;
    virtual void visit(WhileStmt& node) = 0;
    virtual void visit(ForStmt& node) = 0;
    virtual void visit(SwitchStmt& node) = 0;
    virtual void visit(CaseStmt& node) = 0;
    virtual void visit(BreakStmt& node) = 0;
    virtual void visit(ContinueStmt& node) = 0;
    virtual void visit(ReturnStmt& node) = 0;
    virtual void visit(ExprStmt& node) = 0;
    virtual void visit(BinaryExpr& node) = 0;
    virtual void visit(UnaryExpr& node) = 0;
    virtual void visit(CallExpr& node) = 0;
    virtual void visit(StructAccessExpr& node) = 0;
    virtual void visit(LiteralExpr& node) = 0;
    virtual void visit(IdentifierExpr& node) = 0;
    virtual void visit(ListLiteralExpr& node) = 0;
    virtual void visit(ListIndexExpr& node) = 0;
    virtual void visit(ListMethodCallExpr& node) = 0;
    virtual void visit(PreIncrementExpr& node) = 0;
    virtual void visit(PostIncrementExpr& node) = 0;
};

} // namespace ris
