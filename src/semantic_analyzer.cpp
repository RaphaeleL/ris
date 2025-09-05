#include "semantic_analyzer.h"
#include <sstream>
#include <iostream>

namespace ris {

SemanticAnalyzer::SemanticAnalyzer() 
    : has_error_(false), error_message_("") {
    // Add runtime functions to the global scope
    add_runtime_functions();
}

bool SemanticAnalyzer::analyze(Program& program) {
    has_error_ = false;
    error_message_ = "";
    errors_.clear();
    
    analyze_program(program);
    
    return !has_error_;
}

void SemanticAnalyzer::error(const std::string& message, const SourcePos& position) {
    has_error_ = true;
    std::stringstream ss;
    ss << message << " at " << position.line << ":" << position.column;
    std::string full_message = ss.str();
    
    if (error_message_.empty()) {
        error_message_ = full_message;
    }
    
    errors_.push_back(full_message);
}

void SemanticAnalyzer::add_error(const std::string& message) {
    has_error_ = true;
    if (error_message_.empty()) {
        error_message_ = message;
    }
    errors_.push_back(message);
}

std::unique_ptr<Type> SemanticAnalyzer::analyze_type(const std::string& type_name) {
    auto type = create_type(type_name);
    if (!type) {
        error("Unknown type: " + type_name, SourcePos());
    }
    return type;
}

std::unique_ptr<Type> SemanticAnalyzer::analyze_expression_type(Expr& expr) {
    // This is a simplified version - in a real implementation, we'd need to
    // track expression types more carefully
    if (auto* literal = dynamic_cast<LiteralExpr*>(&expr)) {
        // Map literal token types to actual types
        switch (literal->type) {
            case TokenType::INTEGER_LITERAL:
                return create_type("int");
            case TokenType::FLOAT_LITERAL:
                return create_type("float");
            case TokenType::CHAR_LITERAL:
                return create_type("char");
            case TokenType::STRING_LITERAL:
                return create_type("string");
            case TokenType::TRUE:
            case TokenType::FALSE:
                return create_type("bool");
            default:
                return create_type("int"); // Default to int
        }
    } else if (auto* identifier = dynamic_cast<IdentifierExpr*>(&expr)) {
        Symbol* symbol = symbol_table_.lookup(identifier->name);
        if (symbol && symbol->kind() == Symbol::Kind::VARIABLE) {
            auto* var_symbol = static_cast<VariableSymbol*>(symbol);
            // Create a copy of the type
            auto type_name = var_symbol->type().to_string();
            return create_type(type_name);
        }
    } else if (auto* binary = dynamic_cast<BinaryExpr*>(&expr)) {
        // For binary expressions, determine the result type based on the operator
        switch (binary->op) {
            case TokenType::PLUS:
            case TokenType::MINUS:
            case TokenType::MULTIPLY:
            case TokenType::DIVIDE:
            case TokenType::MODULO:
                // Arithmetic operations return the type of the operands
                return analyze_expression_type(*binary->left);
                
            case TokenType::EQUAL:
            case TokenType::NOT_EQUAL:
            case TokenType::LESS:
            case TokenType::GREATER:
            case TokenType::LESS_EQUAL:
            case TokenType::GREATER_EQUAL:
                // Comparison operations return bool
                return create_type("bool");
                
            case TokenType::AND:
            case TokenType::OR:
                // Logical operations return bool
                return create_type("bool");
                
            case TokenType::ASSIGN:
                // Assignment returns the type of the left operand
                return analyze_expression_type(*binary->left);
                
            default:
                return create_type("int");
        }
    } else if (auto* unary = dynamic_cast<UnaryExpr*>(&expr)) {
        // For unary expressions, determine the result type based on the operator
        switch (unary->op) {
            case TokenType::NOT:
                // Logical NOT returns bool
                return create_type("bool");
                
            case TokenType::MINUS:
                // Unary minus returns the type of the operand
                return analyze_expression_type(*unary->operand);
                
            default:
                return create_type("int");
        }
    }
    // For now, return int as default
    return create_type("int");
}

bool SemanticAnalyzer::check_type_compatibility(const Type& expected, const Type& actual, const SourcePos& position) {
    if (expected.equals(actual)) {
        return true;
    }
    
    // Check for implicit conversions
    if (expected.is_assignable_from(actual)) {
        return true;
    }
    
    error("Type mismatch: expected " + expected.to_string() + ", got " + actual.to_string(), position);
    return false;
}

bool SemanticAnalyzer::check_assignable(const Type& target, const Type& source, const SourcePos& position) {
    if (target.is_assignable_from(source)) {
        return true;
    }
    
    error("Cannot assign " + source.to_string() + " to " + target.to_string(), position);
    return false;
}

bool SemanticAnalyzer::check_comparable(const Type& left, const Type& right, const SourcePos& position) {
    if (left.is_comparable_with(right)) {
        return true;
    }
    
    error("Cannot compare " + left.to_string() + " with " + right.to_string(), position);
    return false;
}

bool SemanticAnalyzer::check_arithmetic(const Type& type, const SourcePos& position) {
    if (type.is_arithmetic()) {
        return true;
    }
    
    error("Arithmetic operation requires numeric type, got " + type.to_string(), position);
    return false;
}

bool SemanticAnalyzer::check_boolean(const Type& type, const SourcePos& position) {
    if (type.is_boolean()) {
        return true;
    }
    
    error("Boolean operation requires bool type, got " + type.to_string(), position);
    return false;
}

void SemanticAnalyzer::analyze_program(Program& program) {
    // Analyze global variables first
    for (auto& var : program.globals) {
        analyze_variable_declaration(*var, true);
    }
    
    // Then analyze functions
    for (auto& func : program.functions) {
        analyze_function(*func);
    }
}

void SemanticAnalyzer::analyze_function(FuncDecl& func) {
    // Create function type
    std::vector<std::unique_ptr<Type>> param_types;
    for (const auto& param : func.parameters) {
        auto param_type = analyze_type(param.first);
        if (!param_type) {
            error("Unknown parameter type: " + param.first, func.position);
            return;
        }
        param_types.push_back(std::move(param_type));
    }
    
    auto return_type = analyze_type(func.return_type);
    if (!return_type) {
        error("Unknown return type: " + func.return_type, func.position);
        return;
    }
    
    // Add function to symbol table
    auto func_symbol = std::make_unique<FunctionSymbol>(
        func.name, 
        std::move(return_type),
        std::move(param_types),
        func.position
    );
    
    if (!symbol_table_.add_symbol(std::move(func_symbol))) {
        error("Function '" + func.name + "' already declared", func.position);
        return;
    }
    
    // Enter function scope
    symbol_table_.enter_scope();
    
    // Add parameters to scope
    for (size_t i = 0; i < func.parameters.size(); ++i) {
        const auto& param = func.parameters[i];
        auto param_type = analyze_type(param.first);
        if (param_type) {
            auto param_symbol = std::make_unique<VariableSymbol>(
                param.second, std::move(param_type), func.position
            );
            symbol_table_.add_symbol(std::move(param_symbol));
        }
    }
    
    // Analyze function body
    if (func.body) {
        analyze_block(*func.body);
    }
    
    // Exit function scope
    symbol_table_.exit_scope();
}

void SemanticAnalyzer::analyze_variable_declaration(VarDecl& var, bool is_global) {
    auto var_type = analyze_type(var.type);
    if (!var_type) {
        error("Unknown variable type: " + var.type, var.position);
        return;
    }
    
    // Check if variable already exists
    if (symbol_table_.has_symbol(var.name)) {
        error("Variable '" + var.name + "' already declared", var.position);
        return;
    }
    
    // Create variable symbol
    auto var_symbol = std::make_unique<VariableSymbol>(
        var.name, std::move(var_type), var.position, var.is_array, var.array_size
    );
    
    if (!symbol_table_.add_symbol(std::move(var_symbol))) {
        error("Failed to add variable '" + var.name + "' to symbol table", var.position);
        return;
    }
    
    // Analyze initializer if present
    if (var.initializer) {
        analyze_expression(*var.initializer);
        
        // Check type compatibility
        auto init_type = analyze_expression_type(*var.initializer);
        if (init_type) {
            // Create a new type for comparison since var_type was moved
            auto expected_type = analyze_type(var.type);
            if (expected_type) {
                check_assignable(*expected_type, *init_type, var.position);
            }
        }
    }
}

void SemanticAnalyzer::analyze_statement(Stmt& stmt) {
    if (auto* block = dynamic_cast<BlockStmt*>(&stmt)) {
        analyze_block(*block);
    } else if (auto* if_stmt = dynamic_cast<IfStmt*>(&stmt)) {
        analyze_if_statement(*if_stmt);
    } else if (auto* while_stmt = dynamic_cast<WhileStmt*>(&stmt)) {
        analyze_while_statement(*while_stmt);
    } else if (auto* for_stmt = dynamic_cast<ForStmt*>(&stmt)) {
        analyze_for_statement(*for_stmt);
    } else if (auto* return_stmt = dynamic_cast<ReturnStmt*>(&stmt)) {
        analyze_return_statement(*return_stmt);
    } else if (auto* expr_stmt = dynamic_cast<ExprStmt*>(&stmt)) {
        analyze_expression_statement(*expr_stmt);
    } else if (auto* var_decl = dynamic_cast<VarDecl*>(&stmt)) {
        analyze_variable_declaration(*var_decl);
    }
}

void SemanticAnalyzer::analyze_block(BlockStmt& block) {
    symbol_table_.enter_scope();
    
    for (auto& stmt : block.statements) {
        analyze_statement(*stmt);
    }
    
    symbol_table_.exit_scope();
}

void SemanticAnalyzer::analyze_if_statement(IfStmt& stmt) {
    if (stmt.condition) {
        analyze_expression(*stmt.condition);
        auto cond_type = analyze_expression_type(*stmt.condition);
        if (cond_type) {
            check_boolean(*cond_type, stmt.position);
        }
    }
    
    if (stmt.then_branch) {
        analyze_statement(*stmt.then_branch);
    }
    
    if (stmt.else_branch) {
        analyze_statement(*stmt.else_branch);
    }
}

void SemanticAnalyzer::analyze_while_statement(WhileStmt& stmt) {
    if (stmt.condition) {
        analyze_expression(*stmt.condition);
        auto cond_type = analyze_expression_type(*stmt.condition);
        if (cond_type) {
            check_boolean(*cond_type, stmt.position);
        }
    }
    
    if (stmt.body) {
        analyze_statement(*stmt.body);
    }
}

void SemanticAnalyzer::analyze_for_statement(ForStmt& stmt) {
    symbol_table_.enter_scope();
    
    if (stmt.init) {
        analyze_variable_declaration(*stmt.init);
    }
    
    if (stmt.condition) {
        analyze_expression(*stmt.condition);
        auto cond_type = analyze_expression_type(*stmt.condition);
        if (cond_type) {
            check_boolean(*cond_type, stmt.position);
        }
    }
    
    if (stmt.update) {
        analyze_expression(*stmt.update);
    }
    
    if (stmt.body) {
        analyze_statement(*stmt.body);
    }
    
    symbol_table_.exit_scope();
}

void SemanticAnalyzer::analyze_return_statement(ReturnStmt& stmt) {
    if (stmt.value) {
        analyze_expression(*stmt.value);
        // TODO: Check return type compatibility with function return type
    }
}

void SemanticAnalyzer::analyze_expression_statement(ExprStmt& stmt) {
    if (stmt.expression) {
        analyze_expression(*stmt.expression);
    }
}

void SemanticAnalyzer::analyze_expression(Expr& expr) {
    if (auto* binary = dynamic_cast<BinaryExpr*>(&expr)) {
        if (binary->left && binary->right) {
            analyze_binary_expression(*binary);
        }
    } else if (auto* unary = dynamic_cast<UnaryExpr*>(&expr)) {
        if (unary->operand) {
            analyze_unary_expression(*unary);
        }
    } else if (auto* call = dynamic_cast<CallExpr*>(&expr)) {
        analyze_call_expression(*call);
    } else if (auto* array_access = dynamic_cast<ArrayIndexExpr*>(&expr)) {
        if (array_access->array && array_access->index) {
            analyze_array_access_expression(*array_access);
        }
    } else if (auto* struct_access = dynamic_cast<StructAccessExpr*>(&expr)) {
        if (struct_access->object) {
            analyze_struct_access_expression(*struct_access);
        }
    } else if (auto* literal = dynamic_cast<LiteralExpr*>(&expr)) {
        analyze_literal_expression(*literal);
    } else if (auto* identifier = dynamic_cast<IdentifierExpr*>(&expr)) {
        analyze_identifier_expression(*identifier);
    }
}

void SemanticAnalyzer::analyze_binary_expression(BinaryExpr& expr) {
    if (!expr.left || !expr.right) {
        return;
    }
    
    analyze_expression(*expr.left);
    analyze_expression(*expr.right);
    
    auto left_type = analyze_expression_type(*expr.left);
    auto right_type = analyze_expression_type(*expr.right);
    
    if (!left_type || !right_type) {
        return;
    }
    
    switch (expr.op) {
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
        case TokenType::MODULO:
            check_arithmetic(*left_type, expr.position);
            check_arithmetic(*right_type, expr.position);
            break;
            
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL:
        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER_EQUAL:
            check_comparable(*left_type, *right_type, expr.position);
            break;
            
        case TokenType::AND:
        case TokenType::OR:
            check_boolean(*left_type, expr.position);
            check_boolean(*right_type, expr.position);
            break;
            
        case TokenType::ASSIGN:
            check_assignable(*left_type, *right_type, expr.position);
            break;
            
        default:
            break;
    }
}

void SemanticAnalyzer::analyze_unary_expression(UnaryExpr& expr) {
    if (!expr.operand) {
        return;
    }
    
    analyze_expression(*expr.operand);
    
    auto operand_type = analyze_expression_type(*expr.operand);
    if (!operand_type) {
        return;
    }
    
    switch (expr.op) {
        case TokenType::MINUS:
            check_arithmetic(*operand_type, expr.position);
            break;
            
        case TokenType::NOT:
            check_boolean(*operand_type, expr.position);
            break;
            
        default:
            break;
    }
}

void SemanticAnalyzer::analyze_call_expression(CallExpr& expr) {
    Symbol* symbol = symbol_table_.lookup(expr.function_name);
    if (!symbol || symbol->kind() != Symbol::Kind::FUNCTION) {
        error("Function '" + expr.function_name + "' not found", expr.position);
        return;
    }
    
    auto* func_symbol = static_cast<FunctionSymbol*>(symbol);
    
    // Check argument count
    if (expr.arguments.size() != func_symbol->parameter_types().size()) {
        error("Function '" + expr.function_name + "' expects " + 
              std::to_string(func_symbol->parameter_types().size()) + " arguments, got " + 
              std::to_string(expr.arguments.size()), expr.position);
        return;
    }
    
    // Analyze arguments and check types
    for (size_t i = 0; i < expr.arguments.size(); ++i) {
        analyze_expression(*expr.arguments[i]);
        auto arg_type = analyze_expression_type(*expr.arguments[i]);
        if (arg_type) {
            check_type_compatibility(*func_symbol->parameter_types()[i], *arg_type, expr.position);
        }
    }
}

void SemanticAnalyzer::analyze_array_access_expression(ArrayIndexExpr& expr) {
    analyze_expression(*expr.array);
    analyze_expression(*expr.index);
    
    auto index_type = analyze_expression_type(*expr.index);
    if (index_type) {
        check_arithmetic(*index_type, expr.position);
    }
}

void SemanticAnalyzer::analyze_struct_access_expression(StructAccessExpr& expr) {
    analyze_expression(*expr.object);
    // TODO: Check if object has the field
}

void SemanticAnalyzer::analyze_literal_expression(LiteralExpr& expr) {
    // Literals are always valid
}

void SemanticAnalyzer::analyze_identifier_expression(IdentifierExpr& expr) {
    Symbol* symbol = symbol_table_.lookup(expr.name);
    if (!symbol) {
        error("Undefined variable '" + expr.name + "'", expr.position);
        return;
    }
    
    if (symbol->kind() != Symbol::Kind::VARIABLE) {
        error("'" + expr.name + "' is not a variable", expr.position);
        return;
    }
}

std::string SemanticAnalyzer::get_type_name_from_token(TokenType type) {
    switch (type) {
        case TokenType::INT: return "int";
        case TokenType::FLOAT: return "float";
        case TokenType::BOOL: return "bool";
        case TokenType::CHAR: return "char";
        case TokenType::STRING: return "string";
        case TokenType::VOID: return "void";
        default: return "unknown";
    }
}

bool SemanticAnalyzer::is_type_keyword(TokenType type) {
    return type == TokenType::INT || type == TokenType::FLOAT || 
           type == TokenType::BOOL || type == TokenType::CHAR || 
           type == TokenType::STRING || type == TokenType::VOID;
}

std::unique_ptr<Type> SemanticAnalyzer::create_type_from_token(TokenType type) {
    std::string type_name = get_type_name_from_token(type);
    auto result = create_type(type_name);
    if (!result) {
        error("Unknown type from token: " + type_name, SourcePos());
    }
    return result;
}

void SemanticAnalyzer::add_runtime_functions() {
    // Helper function to add a function symbol
    auto add_func = [this](const std::string& name, const std::string& return_type_name, const std::vector<std::string>& param_type_names) {
        auto return_type = create_type(return_type_name);
        std::vector<std::unique_ptr<Type>> param_types;
        for (const auto& param_type_name : param_type_names) {
            param_types.push_back(create_type(param_type_name));
        }
        auto func_symbol = std::make_unique<FunctionSymbol>(name, std::move(return_type), std::move(param_types), SourcePos());
        symbol_table_.add_symbol(std::move(func_symbol));
    };
    
    // Add all runtime functions
    add_func("ris_print_int", "void", {"int"});
    add_func("ris_print_float", "void", {"float"});
    add_func("ris_print_bool", "void", {"bool"});
    add_func("ris_print_char", "void", {"char"});
    add_func("ris_print_string", "void", {"string"});
    add_func("ris_println_int", "void", {"int"});
    add_func("ris_println_float", "void", {"float"});
    add_func("ris_println_bool", "void", {"bool"});
    add_func("ris_println_char", "void", {"char"});
    add_func("ris_println_string", "void", {"string"});
    add_func("ris_println", "void", {});
    add_func("ris_malloc", "string", {"int"});
    add_func("ris_free", "void", {"string"});
    add_func("ris_string_concat", "string", {"string", "string"});
    add_func("ris_string_length", "int", {"string"});
    add_func("ris_array_alloc", "string", {"int", "int"});
    add_func("ris_array_free", "void", {"string"});
    add_func("ris_exit", "void", {"int"});
}

} // namespace ris
