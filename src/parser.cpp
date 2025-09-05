#include "parser.h"
#include <iostream>

namespace ris {

Parser::Parser(const std::vector<Token>& tokens) 
    : tokens_(tokens), current_token_(0), has_error_(false), error_message_("") {
}

std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();
    
    while (!is_at_end()) {
        if (is_type_keyword(current_token().type)) {
            // Check if it's a function by looking ahead for '('
            size_t lookahead = current_token_;
            while (lookahead < tokens_.size() && 
                   tokens_[lookahead].type != TokenType::LEFT_PAREN && 
                   tokens_[lookahead].type != TokenType::LEFT_BRACE &&
                   tokens_[lookahead].type != TokenType::SEMICOLON) {
                lookahead++;
            }
            
            if (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::LEFT_PAREN) {
                // It's a function
                auto func = parse_function();
                if (func) {
                    program->functions.push_back(std::move(func));
                } else {
                    // If parsing failed, advance to avoid infinite loop
                    advance();
                }
            } else {
                // It's a variable declaration
                auto var = parse_variable_declaration();
                if (var) {
                    program->globals.push_back(std::move(var));
                } else {
                    // If parsing failed, advance to avoid infinite loop
                    advance();
                }
            }
        } else {
            error("Expected declaration");
            break;
        }
    }
    
    return program;
}

Token& Parser::current_token() {
    if (current_token_ >= tokens_.size()) {
        static Token eof_token(TokenType::EOF_TOKEN, "", SourcePos());
        return eof_token;
    }
    return tokens_[current_token_];
}

Token& Parser::peek_token() {
    if (current_token_ + 1 >= tokens_.size()) {
        static Token eof_token(TokenType::EOF_TOKEN, "", SourcePos());
        return eof_token;
    }
    return tokens_[current_token_ + 1];
}

void Parser::advance() {
    if (!is_at_end()) {
        current_token_++;
    }
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) {
    if (is_at_end()) return false;
    return current_token().type == type;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
    } else {
        error(message);
    }
}

void Parser::error(const std::string& message) {
    has_error_ = true;
    if (error_message_.empty()) {
        error_message_ = message + " at " + std::to_string(current_token().position.line) + 
                        ":" + std::to_string(current_token().position.column);
    }
}

bool Parser::is_at_end() const {
    return current_token_ >= tokens_.size() || tokens_[current_token_].type == TokenType::EOF_TOKEN;
}

std::unique_ptr<FuncDecl> Parser::parse_function() {
    // Parse return type
    if (!is_type_keyword(current_token().type)) {
        return nullptr;
    }
    
    std::string return_type = get_type_name(current_token().type);
    advance();
    
    // Parse function name
    if (!check(TokenType::IDENTIFIER)) {
        error("Expected function name");
        return nullptr;
    }
    
    std::string name = current_token().value;
    SourcePos pos = current_token().position;
    advance();
    
    auto func = std::make_unique<FuncDecl>(name, return_type, pos);
    
    // Parse parameters
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (!is_type_keyword(current_token().type)) {
                error("Expected parameter type");
                break;
            }
            
            std::string param_type = get_type_name(current_token().type);
            advance();
            
            if (!check(TokenType::IDENTIFIER)) {
                error("Expected parameter name");
                break;
            }
            
            std::string param_name = current_token().value;
            advance();
            
            func->parameters.push_back({param_type, param_name});
            
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
    
    // Parse function body
    func->body = parse_block();
    
    return func;
}

std::unique_ptr<VarDecl> Parser::parse_variable_declaration() {
    // Parse type
    if (!is_type_keyword(current_token().type)) {
        return nullptr;
    }
    
    std::string type = get_type_name(current_token().type);
    advance();
    
    // Parse variable name
    if (!check(TokenType::IDENTIFIER)) {
        error("Expected variable name");
        return nullptr;
    }
    
    std::string name = current_token().value;
    SourcePos pos = current_token().position;
    advance();
    
    auto var = std::make_unique<VarDecl>(name, type, pos);
    
    // Check for array declaration
    if (match(TokenType::LEFT_BRACKET)) {
        var->is_array = true;
        if (check(TokenType::INTEGER_LITERAL)) {
            var->array_size = std::stoi(current_token().value);
            advance();
        } else {
            var->array_size = -1; // Dynamic array
        }
        consume(TokenType::RIGHT_BRACKET, "Expected ']' after array size");
    }
    
    // Parse initializer
    if (match(TokenType::ASSIGN)) {
        var->initializer = parse_expression();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    
    return var;
}

std::unique_ptr<Stmt> Parser::parse_statement() {
    if (is_type_keyword(current_token().type)) {
        // Variable declaration
        return parse_variable_declaration();
    }
    
    switch (current_token().type) {
        case TokenType::LEFT_BRACE:
            return parse_block();
        case TokenType::IF:
            return parse_if_statement();
        case TokenType::WHILE:
            return parse_while_statement();
        case TokenType::FOR:
            return parse_for_statement();
        case TokenType::RETURN:
            return parse_return_statement();
        case TokenType::BREAK:
        case TokenType::CONTINUE:
            // TODO: Implement break/continue statements
            error("Break/continue statements not yet implemented");
            return nullptr;
        default:
            return parse_expression_statement();
    }
}

std::unique_ptr<BlockStmt> Parser::parse_block() {
    consume(TokenType::LEFT_BRACE, "Expected '{'");
    
    auto block = std::make_unique<BlockStmt>(current_token().position);
    
    while (!check(TokenType::RIGHT_BRACE) && !is_at_end()) {
        auto stmt = parse_statement();
        if (stmt) {
            block->statements.push_back(std::move(stmt));
        } else {
            // Skip to next statement
            advance();
        }
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}'");
    
    return block;
}

std::unique_ptr<IfStmt> Parser::parse_if_statement() {
    consume(TokenType::IF, "Expected 'if'");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    
    auto condition = parse_expression();
    if (!condition) {
        error("Expected condition expression");
        return nullptr;
    }
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition");
    
    auto if_stmt = std::make_unique<IfStmt>(std::move(condition), current_token().position);
    if_stmt->then_branch = parse_statement();
    
    if (match(TokenType::ELSE)) {
        if_stmt->else_branch = parse_statement();
    }
    
    return if_stmt;
}

std::unique_ptr<WhileStmt> Parser::parse_while_statement() {
    consume(TokenType::WHILE, "Expected 'while'");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    
    auto condition = parse_expression();
    if (!condition) {
        error("Expected condition expression");
        return nullptr;
    }
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition");
    
    auto while_stmt = std::make_unique<WhileStmt>(std::move(condition), current_token().position);
    while_stmt->body = parse_statement();
    
    return while_stmt;
}

std::unique_ptr<ForStmt> Parser::parse_for_statement() {
    consume(TokenType::FOR, "Expected 'for'");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'");
    
    auto for_stmt = std::make_unique<ForStmt>(current_token().position);
    
    // Parse initialization
    if (is_type_keyword(current_token().type)) {
        for_stmt->init = parse_variable_declaration();
    } else if (!check(TokenType::SEMICOLON)) {
        for_stmt->init = std::make_unique<VarDecl>("", "", current_token().position);
        for_stmt->init->initializer = parse_expression();
        consume(TokenType::SEMICOLON, "Expected ';' after initialization");
    } else {
        advance(); // consume semicolon
    }
    
    // Parse condition
    if (!check(TokenType::SEMICOLON)) {
        for_stmt->condition = parse_expression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after condition");
    
    // Parse update
    if (!check(TokenType::RIGHT_PAREN)) {
        for_stmt->update = parse_expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after for clause");
    
    // Parse body
    for_stmt->body = parse_statement();
    
    return for_stmt;
}

std::unique_ptr<ReturnStmt> Parser::parse_return_statement() {
    consume(TokenType::RETURN, "Expected 'return'");
    
    auto return_stmt = std::make_unique<ReturnStmt>(current_token().position);
    
    if (!check(TokenType::SEMICOLON)) {
        return_stmt->value = parse_expression();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after return statement");
    
    return return_stmt;
}

std::unique_ptr<ExprStmt> Parser::parse_expression_statement() {
    auto expr = parse_expression();
    if (!expr) {
        return nullptr;
    }
    
    auto stmt = std::make_unique<ExprStmt>(std::move(expr), current_token().position);
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    
    return stmt;
}

std::unique_ptr<Expr> Parser::parse_expression() {
    return parse_assignment();
}

std::unique_ptr<Expr> Parser::parse_assignment() {
    auto expr = parse_logical_or();
    
    if (match(TokenType::ASSIGN)) {
        auto right = parse_assignment();
        if (!right) {
            error("Expected expression after '='");
            return nullptr;
        }
        return std::make_unique<BinaryExpr>(std::move(expr), std::move(right), 
                                           TokenType::ASSIGN, current_token().position);
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parse_logical_or() {
    auto expr = parse_logical_and();
    
    while (match(TokenType::OR)) {
        auto right = parse_logical_and();
        if (!right) {
            error("Expected expression after '||'");
            return nullptr;
        }
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(right), 
                                           TokenType::OR, current_token().position);
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parse_logical_and() {
    auto expr = parse_equality();
    
    while (match(TokenType::AND)) {
        auto right = parse_equality();
        if (!right) {
            error("Expected expression after '&&'");
            return nullptr;
        }
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(right), 
                                           TokenType::AND, current_token().position);
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parse_equality() {
    auto expr = parse_comparison();
    
    while (match(TokenType::EQUAL) || match(TokenType::NOT_EQUAL)) {
        TokenType op = tokens_[current_token_ - 1].type;
        auto right = parse_comparison();
        if (!right) {
            error("Expected expression after operator");
            return nullptr;
        }
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(right), 
                                           op, current_token().position);
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parse_comparison() {
    auto expr = parse_term();
    
    while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) ||
           match(TokenType::LESS) || match(TokenType::LESS_EQUAL)) {
        TokenType op = tokens_[current_token_ - 1].type;
        auto right = parse_term();
        if (!right) {
            error("Expected expression after operator");
            return nullptr;
        }
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(right), 
                                           op, current_token().position);
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parse_term() {
    auto expr = parse_factor();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        TokenType op = tokens_[current_token_ - 1].type;
        auto right = parse_factor();
        if (!right) {
            error("Expected expression after operator");
            return nullptr;
        }
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(right), 
                                           op, current_token().position);
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parse_factor() {
    auto expr = parse_unary();
    
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE) || match(TokenType::MODULO)) {
        TokenType op = tokens_[current_token_ - 1].type;
        auto right = parse_unary();
        if (!right) {
            error("Expected expression after operator");
            return nullptr;
        }
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(right), 
                                           op, current_token().position);
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parse_unary() {
    if (match(TokenType::NOT) || match(TokenType::MINUS)) {
        TokenType op = tokens_[current_token_ - 1].type;
        auto operand = parse_unary();
        if (!operand) {
            error("Expected expression after unary operator");
            return nullptr;
        }
        return std::make_unique<UnaryExpr>(std::move(operand), op, current_token().position);
    }
    
    return parse_primary();
}

std::unique_ptr<Expr> Parser::parse_primary() {
    if (is_literal(current_token().type)) {
        auto literal = std::make_unique<LiteralExpr>(current_token().value, 
                                                    current_token().type, 
                                                    current_token().position);
        advance();
        return literal;
    }
    
    if (match(TokenType::IDENTIFIER)) {
        std::string name = tokens_[current_token_ - 1].value;
        
        if (check(TokenType::LEFT_PAREN)) {
            // Function call
            return parse_call();
        } else if (check(TokenType::LEFT_BRACKET)) {
            // Array access
            return parse_array_access();
        } else if (check(TokenType::DOT)) {
            // Struct access
            return parse_struct_access();
        } else {
            // Simple identifier
            return std::make_unique<IdentifierExpr>(name, current_token().position);
        }
    }
    
    if (match(TokenType::LEFT_PAREN)) {
        auto expr = parse_expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return expr;
    }
    
    error("Expected expression");
    return nullptr;
}

std::unique_ptr<CallExpr> Parser::parse_call() {
    std::string name = tokens_[current_token_ - 1].value;
    auto call = std::make_unique<CallExpr>(name, current_token().position);
    
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            auto arg = parse_expression();
            if (arg) {
                call->arguments.push_back(std::move(arg));
            } else {
                error("Expected expression argument");
                break;
            }
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
    
    return call;
}

std::unique_ptr<ArrayIndexExpr> Parser::parse_array_access() {
    auto array = std::make_unique<IdentifierExpr>(tokens_[current_token_ - 1].value, 
                                                 current_token().position);
    
    consume(TokenType::LEFT_BRACKET, "Expected '[' after array name");
    auto index = parse_expression();
    if (!index) {
        error("Expected index expression");
        return nullptr;
    }
    consume(TokenType::RIGHT_BRACKET, "Expected ']' after index");
    
    return std::make_unique<ArrayIndexExpr>(std::move(array), std::move(index), 
                                           current_token().position);
}

std::unique_ptr<StructAccessExpr> Parser::parse_struct_access() {
    auto object = std::make_unique<IdentifierExpr>(tokens_[current_token_ - 1].value, 
                                                  current_token().position);
    
    consume(TokenType::DOT, "Expected '.' after object name");
    
    if (!check(TokenType::IDENTIFIER)) {
        error("Expected field name");
        return nullptr;
    }
    
    std::string field_name = current_token().value;
    advance();
    
    return std::make_unique<StructAccessExpr>(std::move(object), field_name, 
                                             current_token().position);
}

// Utility methods
std::string Parser::get_type_name(TokenType type) {
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

bool Parser::is_type_keyword(TokenType type) {
    return type == TokenType::INT || type == TokenType::FLOAT || 
           type == TokenType::BOOL || type == TokenType::CHAR || 
           type == TokenType::STRING || type == TokenType::VOID;
}

bool Parser::is_literal(TokenType type) {
    return type == TokenType::INTEGER_LITERAL || type == TokenType::FLOAT_LITERAL ||
           type == TokenType::CHAR_LITERAL || type == TokenType::STRING_LITERAL ||
           type == TokenType::TRUE || type == TokenType::FALSE;
}

bool Parser::is_operator(TokenType type) {
    return type == TokenType::PLUS || type == TokenType::MINUS ||
           type == TokenType::MULTIPLY || type == TokenType::DIVIDE ||
           type == TokenType::MODULO || type == TokenType::EQUAL ||
           type == TokenType::NOT_EQUAL || type == TokenType::LESS ||
           type == TokenType::GREATER || type == TokenType::LESS_EQUAL ||
           type == TokenType::GREATER_EQUAL || type == TokenType::AND ||
           type == TokenType::OR || type == TokenType::NOT ||
           type == TokenType::ASSIGN;
}

int Parser::get_operator_precedence(TokenType op) {
    switch (op) {
        case TokenType::OR: return 1;
        case TokenType::AND: return 2;
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL: return 3;
        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER_EQUAL: return 4;
        case TokenType::PLUS:
        case TokenType::MINUS: return 5;
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
        case TokenType::MODULO: return 6;
        case TokenType::NOT: return 7;
        case TokenType::ASSIGN: return 0; // Right associative
        default: return -1;
    }
}

bool Parser::is_right_associative(TokenType op) {
    return op == TokenType::ASSIGN;
}

} // namespace ris
