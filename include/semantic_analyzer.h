#pragma once

#include "ast.h"
#include "types.h"
#include "symbol_table.h"
#include <string>
#include <vector>

namespace ris {

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    
    // Main analysis method
    bool analyze(std::unique_ptr<Program> program);
    
    // Check if there were any errors
    bool has_error() const { return has_error_; }
    const std::string& error_message() const { return error_message_; }
    const std::vector<std::string>& errors() const { return errors_; }

private:
    SymbolTable symbol_table_;
    bool has_error_;
    std::string error_message_;
    std::vector<std::string> errors_;
    
    // Helper methods
    void error(const std::string& message, const SourcePos& position);
    void add_error(const std::string& message);
    
    // Type analysis
    std::unique_ptr<Type> analyze_type(const std::string& type_name);
    std::unique_ptr<Type> analyze_expression_type(Expr& expr);
    bool check_type_compatibility(const Type& expected, const Type& actual, const SourcePos& position);
    bool check_assignable(const Type& target, const Type& source, const SourcePos& position);
    bool check_comparable(const Type& left, const Type& right, const SourcePos& position);
    bool check_arithmetic(const Type& type, const SourcePos& position);
    bool check_boolean(const Type& type, const SourcePos& position);
    
    // Program analysis
    void analyze_program(Program& program);
    void analyze_function(FuncDecl& func);
    void analyze_variable_declaration(VarDecl& var, bool is_global = false);
    void analyze_statement(Stmt& stmt);
    void analyze_block(BlockStmt& block);
    void analyze_if_statement(IfStmt& stmt);
    void analyze_while_statement(WhileStmt& stmt);
    void analyze_for_statement(ForStmt& stmt);
    void analyze_return_statement(ReturnStmt& stmt);
    void analyze_expression_statement(ExprStmt& stmt);
    
    // Expression analysis
    void analyze_expression(Expr& expr);
    void analyze_binary_expression(BinaryExpr& expr);
    void analyze_unary_expression(UnaryExpr& expr);
    void analyze_call_expression(CallExpr& expr);
    void analyze_array_access_expression(ArrayIndexExpr& expr);
    void analyze_struct_access_expression(StructAccessExpr& expr);
    void analyze_literal_expression(LiteralExpr& expr);
    void analyze_identifier_expression(IdentifierExpr& expr);
    
    // Utility methods
    std::string get_type_name_from_token(TokenType type);
    bool is_type_keyword(TokenType type);
    std::unique_ptr<Type> create_type_from_token(TokenType type);
};

} // namespace ris
