#pragma once

#include "ast.h"
#include "types.h"
#include "diagnostics.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <memory>
#include <string>
#include <map>

namespace ris {

class CodeGenerator {
public:
    CodeGenerator();
    ~CodeGenerator();
    
    // Main entry point
    bool generate(std::unique_ptr<Program> program, const std::string& output_file);
    
    // Error handling
    bool has_error() const { return has_error_; }
    const std::string& error_message() const { return error_message_; }
    
    // Get the diagnostic reporter
    DiagnosticReporter& get_diagnostics() { return diagnostics_; }
    const DiagnosticReporter& get_diagnostics() const { return diagnostics_; }
    
private:
    // LLVM context and modules
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    
    // Error handling
    bool has_error_;
    std::string error_message_;
    DiagnosticReporter diagnostics_;
    
    // Symbol table for code generation
    std::map<std::string, llvm::Value*> named_values_;
    std::map<std::string, llvm::Function*> functions_;
    
    // Control flow context for break/continue
    struct ControlFlowContext {
        llvm::BasicBlock* break_target;
        llvm::BasicBlock* continue_target;
    };
    std::vector<ControlFlowContext> control_flow_stack_;
    
    // Helper methods
    void error(const std::string& message);
    void error(const std::string& message, const SourcePos& position);
    std::string parse_verification_error(const std::string& error);
    
    // Type conversion
    llvm::Type* get_llvm_type(const Type& type);
    llvm::Type* get_llvm_type(const std::string& type_name);
    
    // Program generation
    void generate_program(Program& program);
    void generate_function(FuncDecl& func);
    void generate_variable_declaration(VarDecl& var, bool is_global = false);
    void generate_statement(Stmt& stmt);
    void generate_block(BlockStmt& block);
    
    // Expression generation
    llvm::Value* generate_expression(Expr& expr);
    llvm::Value* generate_literal_expression(LiteralExpr& expr);
    llvm::Value* generate_identifier_expression(IdentifierExpr& expr);
    llvm::Value* generate_binary_expression(BinaryExpr& expr);
    llvm::Value* generate_unary_expression(UnaryExpr& expr);
    llvm::Value* generate_call_expression(CallExpr& expr);
    llvm::Value* generate_generic_print_call(CallExpr& expr);
    llvm::Value* generate_struct_access_expression(StructAccessExpr& expr);
    llvm::Value* generate_list_literal_expression(ListLiteralExpr& expr);
    llvm::Value* generate_list_index_expression(ListIndexExpr& expr);
    llvm::Value* generate_list_method_call_expression(ListMethodCallExpr& expr);
    llvm::Value* generate_pre_increment_expression(PreIncrementExpr& expr);
    llvm::Value* generate_post_increment_expression(PostIncrementExpr& expr);
    
    // Control flow generation
    void generate_if_statement(IfStmt& stmt);
    void generate_while_statement(WhileStmt& stmt);
    void generate_for_statement(ForStmt& stmt);
    void generate_switch_statement(SwitchStmt& stmt);
    void generate_case_statement(CaseStmt& stmt);
    void generate_break_statement(BreakStmt& stmt);
    void generate_continue_statement(ContinueStmt& stmt);
    void generate_return_statement(ReturnStmt& stmt);
    
    // Utility methods
    llvm::Value* create_constant(const std::string& value, const std::string& type);
    void create_main_function();
    void declare_runtime_functions();
};

} // namespace ris
