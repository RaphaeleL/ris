#include "semantic_analyzer.h"
#include "types.h"
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
    
    // Also add to centralized diagnostic system
    diagnostics_.add_error(message, position, "semantic");
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
                // String concatenation returns string, arithmetic returns left operand type
                if (auto left_type = analyze_expression_type(*binary->left)) {
                    if (left_type->to_string() == "string") {
                        return left_type; // String concatenation returns string
                    }
                }
                // Fall through to arithmetic case
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
        return create_type("int"); // Default fallback
    } else if (auto* call = dynamic_cast<CallExpr*>(&expr)) {
        // For function calls, return the return type of the function
        Symbol* symbol = symbol_table_.lookup(call->function_name);
        if (symbol && symbol->kind() == Symbol::Kind::FUNCTION) {
            auto* func_symbol = static_cast<FunctionSymbol*>(symbol);
            return create_type(func_symbol->return_type().to_string());
        }
        return create_type("int"); // Default fallback
    } else if (auto* list_literal = dynamic_cast<ListLiteralExpr*>(&expr)) {
        // For list literals, determine the type from the first element
        if (!list_literal->elements.empty()) {
            auto element_type = analyze_expression_type(*list_literal->elements[0]);
            if (element_type) {
                return create_list_type(std::move(element_type));
            }
        }
        // Empty list - default to list<int>
        return create_list_type(create_type("int"));
    } else if (auto* pre_inc = dynamic_cast<PreIncrementExpr*>(&expr)) {
        // Pre-increment returns the type of the operand
        return analyze_expression_type(*pre_inc->operand);
    } else if (auto* post_inc = dynamic_cast<PostIncrementExpr*>(&expr)) {
        // Post-increment returns the type of the operand
        return analyze_expression_type(*post_inc->operand);
    } else if (auto* list_index = dynamic_cast<ListIndexExpr*>(&expr)) {
        // List indexing returns the element type of the list
        auto list_type = analyze_expression_type(*list_index->list);
        if (auto* list_type_ptr = dynamic_cast<const ListType*>(list_type.get())) {
            // Create a copy of the element type
            auto element_type_name = list_type_ptr->element_type().to_string();
            return create_type(element_type_name);
        }
        return create_type("int"); // Default fallback
    } else if (auto* list_method = dynamic_cast<ListMethodCallExpr*>(&expr)) {
        // For list method calls, determine return type based on method
        if (list_method->method_name == "get") {
            // get() returns the element type of the list
            auto list_type = analyze_expression_type(*list_method->list);
            if (auto* list_type_ptr = dynamic_cast<const ListType*>(list_type.get())) {
                auto element_type_name = list_type_ptr->element_type().to_string();
                return create_type(element_type_name);
            }
            return create_type("int"); // Default fallback
        } else if (list_method->method_name == "size") {
            // size() returns int
            return create_type("int");
        } else if (list_method->method_name == "push" || list_method->method_name == "pop") {
            // push() and pop() return void
            return create_type("void");
        }
        return create_type("int"); // Default fallback
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
    
    // Track current function for return statement analysis
    current_function_name_ = func.name;
    current_function_return_type_ = func.return_type;
    
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
    
    // Check if function has return statement for non-void functions
    if (!func.return_type.empty() && func.return_type != "void") {
        // TODO: This is a simplified check - in a real implementation, 
        // we'd analyze the control flow to ensure all paths return a value
        // For now, we'll let the codegen handle this
    }
    
    // Exit function scope
    symbol_table_.exit_scope();
    
    // Clear current function tracking
    current_function_name_.clear();
    current_function_return_type_.clear();
}

void SemanticAnalyzer::analyze_variable_declaration(VarDecl& var, bool /* is_global */) {
    std::unique_ptr<Type> var_type;
    
    // Regular variable type
    var_type = analyze_type(var.type);
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
        var.name, std::move(var_type), var.position
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
    } else if (auto* switch_stmt = dynamic_cast<SwitchStmt*>(&stmt)) {
        analyze_switch_statement(*switch_stmt);
    } else if (auto* case_stmt = dynamic_cast<CaseStmt*>(&stmt)) {
        analyze_case_statement(*case_stmt);
    } else if (auto* break_stmt = dynamic_cast<BreakStmt*>(&stmt)) {
        analyze_break_statement(*break_stmt);
    } else if (auto* continue_stmt = dynamic_cast<ContinueStmt*>(&stmt)) {
        analyze_continue_statement(*continue_stmt);
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
        
        // Check return type compatibility with function return type
        auto return_type = analyze_expression_type(*stmt.value);
        if (return_type && !current_function_return_type_.empty()) {
            auto func_return_type = create_type(current_function_return_type_);
            if (func_return_type) {
                if (func_return_type->is_void() && !return_type->is_void()) {
                    error("Function declared as 'void' cannot return a value. Remove the return statement or change function return type.", stmt.position);
                } else if (!func_return_type->is_void() && return_type->is_void()) {
                    error("Function must return a value. Add a return statement or change function return type to 'void'.", stmt.position);
                } else if (!func_return_type->is_void() && !return_type->is_void()) {
                    check_assignable(*func_return_type, *return_type, stmt.position);
                }
            }
        }
    } else {
        // No return value - check if function expects a return value
        if (!current_function_return_type_.empty()) {
            auto func_return_type = create_type(current_function_return_type_);
            if (func_return_type && !func_return_type->is_void()) {
                error("Function must return a value. Add a return statement or change function return type to 'void'.", stmt.position);
            }
        }
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
    } else if (auto* struct_access = dynamic_cast<StructAccessExpr*>(&expr)) {
        if (struct_access->object) {
            analyze_struct_access_expression(*struct_access);
        }
    } else if (auto* literal = dynamic_cast<LiteralExpr*>(&expr)) {
        analyze_literal_expression(*literal);
    } else if (auto* identifier = dynamic_cast<IdentifierExpr*>(&expr)) {
        analyze_identifier_expression(*identifier);
    } else if (auto* list_literal = dynamic_cast<ListLiteralExpr*>(&expr)) {
        analyze_list_literal_expression(*list_literal);
    } else if (auto* list_index = dynamic_cast<ListIndexExpr*>(&expr)) {
        analyze_list_index_expression(*list_index);
    } else if (auto* list_method = dynamic_cast<ListMethodCallExpr*>(&expr)) {
        analyze_list_method_call_expression(*list_method);
    } else if (auto* pre_inc = dynamic_cast<PreIncrementExpr*>(&expr)) {
        analyze_pre_increment_expression(*pre_inc);
    } else if (auto* post_inc = dynamic_cast<PostIncrementExpr*>(&expr)) {
        analyze_post_increment_expression(*post_inc);
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
            // Allow string concatenation: string + string
            if (left_type->to_string() == "string" && right_type->to_string() == "string") {
                // String concatenation is allowed
                break;
            }
            // Fall through to arithmetic check for numeric types
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
    // Handle print functions specially
    if (expr.function_name == "print" || expr.function_name == "println") {
        // For print/println, allow any number of arguments of any type
        for (auto& arg : expr.arguments) {
            analyze_expression(*arg);
            // No type checking - accept any type
        }
        return;
    }
    
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


void SemanticAnalyzer::analyze_struct_access_expression(StructAccessExpr& expr) {
    analyze_expression(*expr.object);
    // TODO: Check if object has the field
}

void SemanticAnalyzer::analyze_literal_expression(LiteralExpr& /* expr */) {
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
        case TokenType::LIST: return "list";
        default: return "unknown";
    }
}

bool SemanticAnalyzer::is_type_keyword(TokenType type) {
    return type == TokenType::INT || type == TokenType::FLOAT || 
           type == TokenType::BOOL || type == TokenType::CHAR || 
           type == TokenType::STRING || type == TokenType::VOID ||
           type == TokenType::LIST;
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
    // Print functions (like Python's print) - register as special functions
    add_func("print", "void", {"int"}); // This will be the only one registered, but we'll handle it specially
    add_func("println", "void", {"int"}); // This will be the only one registered, but we'll handle it specially
    
    add_func("ris_malloc", "string", {"int"});
    add_func("ris_free", "void", {"string"});
    add_func("ris_string_concat", "string", {"string", "string"});
    add_func("ris_string_length", "int", {"string"});
    add_func("ris_exit", "void", {"int"});
}

void SemanticAnalyzer::analyze_switch_statement(SwitchStmt& stmt) {
    if (stmt.expression) {
        analyze_expression(*stmt.expression);
        auto expr_type = analyze_expression_type(*stmt.expression);
        if (expr_type) {
            // Switch expression should be comparable (int, char, etc.)
            if (!expr_type->is_arithmetic() && !expr_type->is_boolean()) {
                error("Switch expression must be of arithmetic or boolean type", stmt.position);
            }
        }
    }
    
    // Analyze all cases
    for (auto& case_stmt : stmt.cases) {
        if (case_stmt) {
            analyze_case_statement(*case_stmt);
        }
    }
}

void SemanticAnalyzer::analyze_case_statement(CaseStmt& stmt) {
    if (stmt.value) {
        // This is a case with a value, not default
        analyze_expression(*stmt.value);
        auto case_type = analyze_expression_type(*stmt.value);
        if (case_type) {
            // Case value should be comparable with switch expression
            // For now, we'll just check it's a valid type
            if (!case_type->is_arithmetic() && !case_type->is_boolean()) {
                error("Case value must be of arithmetic or boolean type", stmt.position);
            }
        }
    }
    
    // Analyze statements in the case
    for (auto& stmt_ptr : stmt.statements) {
        if (stmt_ptr) {
            analyze_statement(*stmt_ptr);
        }
    }
}

void SemanticAnalyzer::analyze_break_statement(BreakStmt& /* stmt */) {
    // Break statements are valid in switch, while, and for loops
    // For now, we'll just accept them - proper validation would require
    // tracking the current control structure context
}

void SemanticAnalyzer::analyze_continue_statement(ContinueStmt& /* stmt */) {
    // Continue statements are valid in while and for loops
    // For now, we'll just accept them - proper validation would require
    // tracking the current control structure context
}

void SemanticAnalyzer::analyze_list_literal_expression(ListLiteralExpr& expr) {
    // Analyze all elements in the list
    for (auto& element : expr.elements) {
        if (element) {
            analyze_expression(*element);
        }
    }
    
    // Check that all elements have the same type
    if (!expr.elements.empty()) {
        auto first_type = analyze_expression_type(*expr.elements[0]);
        for (size_t i = 1; i < expr.elements.size(); ++i) {
            auto element_type = analyze_expression_type(*expr.elements[i]);
            if (first_type && element_type && !first_type->equals(*element_type)) {
                error("List elements must all be of the same type", expr.position);
                break;
            }
        }
    }
}

void SemanticAnalyzer::analyze_list_index_expression(ListIndexExpr& expr) {
    if (expr.list) {
        analyze_expression(*expr.list);
    }
    if (expr.index) {
        analyze_expression(*expr.index);
    }
    
    // Check that the list is actually a list type
    auto list_type = analyze_expression_type(*expr.list);
    if (list_type && !dynamic_cast<const ListType*>(list_type.get())) {
        error("Indexing operator '[]' can only be used on lists", expr.position);
    }
    
    // Check that the index is an integer
    auto index_type = analyze_expression_type(*expr.index);
    if (index_type && !index_type->is_arithmetic()) {
        error("List index must be an integer", expr.position);
    }
}

void SemanticAnalyzer::analyze_list_method_call_expression(ListMethodCallExpr& expr) {
    if (expr.list) {
        analyze_expression(*expr.list);
    }
    
    // Check that the list is actually a list type
    auto list_type = analyze_expression_type(*expr.list);
    if (list_type && !dynamic_cast<const ListType*>(list_type.get())) {
        error("Method calls can only be used on lists", expr.position);
        return;
    }
    
    // Analyze arguments
    for (auto& arg : expr.arguments) {
        if (arg) {
            analyze_expression(*arg);
        }
    }
    
    // Validate method calls
    if (expr.method_name == "push") {
        if (expr.arguments.size() != 1) {
            error("push() method requires exactly one argument", expr.position);
        } else {
            // Check that the argument type matches the list element type
            auto list_type_ptr = dynamic_cast<const ListType*>(list_type.get());
            if (list_type_ptr) {
                auto arg_type = analyze_expression_type(*expr.arguments[0]);
                if (arg_type && !list_type_ptr->element_type().is_assignable_from(*arg_type)) {
                    error("push() argument type must match list element type", expr.position);
                }
            }
        }
    } else if (expr.method_name == "get") {
        if (expr.arguments.empty()) {
            error("get() method requires at least one index argument", expr.position);
        } else {
            // Check that all arguments are integers
            for (size_t i = 0; i < expr.arguments.size(); ++i) {
                auto arg_type = analyze_expression_type(*expr.arguments[i]);
                if (arg_type && !arg_type->is_arithmetic()) {
                    error("get() index arguments must be integers", expr.position);
                }
            }
        }
    } else if (expr.method_name == "pop" || expr.method_name == "size") {
        if (!expr.arguments.empty()) {
            error(expr.method_name + "() method takes no arguments", expr.position);
        }
    } else {
        error("Unknown list method: " + expr.method_name, expr.position);
    }
}

void SemanticAnalyzer::analyze_pre_increment_expression(PreIncrementExpr& expr) {
    if (!expr.operand) {
        error("Expected operand for pre-increment", expr.position);
        return;
    }
    
    analyze_expression(*expr.operand);
    
    // Check that the operand is a variable (not a literal or complex expression)
    if (!dynamic_cast<IdentifierExpr*>(expr.operand.get())) {
        error("Pre-increment operand must be a variable", expr.position);
        return;
    }
    
    // Check that the operand is an integer type
    auto operand_type = analyze_expression_type(*expr.operand);
    if (!operand_type || !operand_type->is_arithmetic()) {
        error("Pre-increment operand must be an integer", expr.position);
        return;
    }
    
    // The expression type is determined by analyze_expression_type
}

void SemanticAnalyzer::analyze_post_increment_expression(PostIncrementExpr& expr) {
    if (!expr.operand) {
        error("Expected operand for post-increment", expr.position);
        return;
    }
    
    analyze_expression(*expr.operand);
    
    // Check that the operand is a variable (not a literal or complex expression)
    if (!dynamic_cast<IdentifierExpr*>(expr.operand.get())) {
        error("Post-increment operand must be a variable", expr.position);
        return;
    }
    
    // Check that the operand is an integer type
    auto operand_type = analyze_expression_type(*expr.operand);
    if (!operand_type || !operand_type->is_arithmetic()) {
        error("Post-increment operand must be an integer", expr.position);
        return;
    }
    
    // The expression type is determined by analyze_expression_type
}

} // namespace ris
