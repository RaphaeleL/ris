#pragma once

#include "lexer.h"
#include "ast.h"
#include <vector>

namespace ris {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    
    // Main parsing method
    std::unique_ptr<Program> parse();
    
    // Check if there were any errors
    bool has_error() const { return has_error_; }
    const std::string& error_message() const { return error_message_; }
    
    // Check if we're at the end of tokens
    bool is_at_end() const;

private:
    std::vector<Token> tokens_;
    size_t current_token_;
    bool has_error_;
    std::string error_message_;
    
    // Helper methods
    Token& current_token();
    Token& peek_token();
    void advance();
    bool match(TokenType type);
    bool check(TokenType type);
    void consume(TokenType type, const std::string& message);
    void error(const std::string& message);
    
    // Parsing methods
    std::unique_ptr<Program> parse_program();
    std::unique_ptr<FuncDecl> parse_function();
    std::unique_ptr<VarDecl> parse_variable_declaration();
    std::unique_ptr<Stmt> parse_statement();
    std::unique_ptr<BlockStmt> parse_block();
    std::unique_ptr<IfStmt> parse_if_statement();
    std::unique_ptr<WhileStmt> parse_while_statement();
    std::unique_ptr<ForStmt> parse_for_statement();
    std::unique_ptr<ReturnStmt> parse_return_statement();
    std::unique_ptr<ExprStmt> parse_expression_statement();
    std::unique_ptr<Expr> parse_expression();
    std::unique_ptr<Expr> parse_assignment();
    std::unique_ptr<Expr> parse_logical_or();
    std::unique_ptr<Expr> parse_logical_and();
    std::unique_ptr<Expr> parse_equality();
    std::unique_ptr<Expr> parse_comparison();
    std::unique_ptr<Expr> parse_term();
    std::unique_ptr<Expr> parse_factor();
    std::unique_ptr<Expr> parse_unary();
    std::unique_ptr<Expr> parse_primary();
    std::unique_ptr<CallExpr> parse_call();
    std::unique_ptr<ArrayIndexExpr> parse_array_access();
    std::unique_ptr<StructAccessExpr> parse_struct_access();
    
    // Utility methods
    std::string get_type_name(TokenType type);
    bool is_type_keyword(TokenType type);
    bool is_literal(TokenType type);
    bool is_operator(TokenType type);
    int get_operator_precedence(TokenType op);
    bool is_right_associative(TokenType op);
};

} // namespace ris
