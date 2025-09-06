#include "parser.h"
#include "lexer.h"
#include <iostream>
#include <cassert>
#include "test_utils.h"

// Simple test framework
#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            std::cerr << " FAIL  " << #expected << " != " << #actual << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while(0)

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << " FAIL  " << #condition << " is false at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            std::cerr << " FAIL  " << #condition << " is true at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while(0)

int test_parser_basic_function() {
    std::cout << "Running test_parser_basic_function .........";
    
    ris::Lexer lexer("int main() { return 42; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    ASSERT_EQ(0, program->globals.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ("int", func->return_type);
    ASSERT_EQ(0, func->parameters.size());
    ASSERT_TRUE(func->body != nullptr);
    ASSERT_EQ(1, func->body->statements.size());
    
    
    return 0;
}

int test_parser_function_with_parameters() {
    std::cout << "Running test_parser_function_with_parameters .........";
    
    ris::Lexer lexer("int add(int a, int b) { return a + b; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("add", func->name);
    ASSERT_EQ("int", func->return_type);
    ASSERT_EQ(2, func->parameters.size());
    ASSERT_EQ("int", func->parameters[0].first);
    ASSERT_EQ("a", func->parameters[0].second);
    ASSERT_EQ("int", func->parameters[1].first);
    ASSERT_EQ("b", func->parameters[1].second);
    
    
    return 0;
}

int test_parser_variable_declaration() {
    std::cout << "Running test_parser_variable_declaration .........";
    
    ris::Lexer lexer("int x = 42; int y;");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(0, program->functions.size());
    ASSERT_EQ(2, program->globals.size());
    
    auto& var1 = program->globals[0];
    ASSERT_EQ("x", var1->name);
    ASSERT_EQ("int", var1->type);
    ASSERT_TRUE(var1->initializer != nullptr);
    
    auto& var2 = program->globals[1];
    ASSERT_EQ("y", var2->name);
    ASSERT_EQ("int", var2->type);
    ASSERT_TRUE(var2->initializer == nullptr);
    
    
    return 0;
}


int test_parser_arithmetic_expressions() {
    std::cout << "Running test_parser_arithmetic_expressions .........";
    
    ris::Lexer lexer("int main() { int x = 1 + 2 * 3; int y = (4 - 2) / 2; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(2, func->body->statements.size());
    
    
    return 0;
}

int test_parser_logical_expressions() {
    std::cout << "Running test_parser_logical_expressions .........";
    
    ris::Lexer lexer("int main() { bool x = true && false; bool y = a > b || c < d; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(2, func->body->statements.size());
    
    
    return 0;
}

int test_parser_if_statement() {
    std::cout << "Running test_parser_if_statement .........";
    
    ris::Lexer lexer("int main() { if (x > 0) { return 1; } else { return 0; } }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(1, func->body->statements.size());
    
    
    return 0;
}

int test_parser_while_statement() {
    std::cout << "Running test_parser_while_statement .........";
    
    ris::Lexer lexer("int main() { while (x > 0) { x = x - 1; } }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(1, func->body->statements.size());
    
    
    return 0;
}

int test_parser_for_statement() {
    std::cout << "Running test_parser_for_statement .........";
    
    ris::Lexer lexer("int main() { for (int i = 0; i < 10; i = i + 1) { x = x + i; } }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(1, func->body->statements.size());
    
    
    return 0;
}

int test_parser_return_statement() {
    std::cout << "Running test_parser_return_statement .........";
    
    ris::Lexer lexer("int main() { return 42; } void func() { return; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(2, program->functions.size());
    
    auto& func1 = program->functions[0];
    ASSERT_EQ("int", func1->return_type);
    ASSERT_EQ(1, func1->body->statements.size());
    
    auto& func2 = program->functions[1];
    ASSERT_EQ("void", func2->return_type);
    ASSERT_EQ(1, func2->body->statements.size());
    
    
    return 0;
}

int test_parser_function_call() {
    std::cout << "Running test_parser_function_call .........";
    
    ris::Lexer lexer("int main() { int x = add(1, 2); int y = func(); }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(2, func->body->statements.size());
    
    
    return 0;
}


int test_parser_complex_program() {
    std::cout << "Running test_parser_complex_program .........";
    
    ris::Lexer lexer(R"(
        int global_var = 42;
        
        int factorial(int n) {
            if (n <= 1) {
                return 1;
            } else {
                return n * factorial(n - 1);
            }
        }
        
        int main() {
            int x = 5;
            int result = factorial(x);
            return result;
        }
    )");
    
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->globals.size());
    ASSERT_EQ(2, program->functions.size());
    
    auto& global = program->globals[0];
    ASSERT_EQ("global_var", global->name);
    ASSERT_EQ("int", global->type);
    
    auto& factorial = program->functions[0];
    ASSERT_EQ("factorial", factorial->name);
    ASSERT_EQ("int", factorial->return_type);
    ASSERT_EQ(1, factorial->parameters.size());
    
    auto& main = program->functions[1];
    ASSERT_EQ("main", main->name);
    ASSERT_EQ("int", main->return_type);
    ASSERT_EQ(0, main->parameters.size());
    
    
    return 0;
}

int test_parser_error_handling() {
    std::cout << "Running test_parser_error_handling .........";
    
    // Test missing semicolon
    ris::Lexer lexer1("int main() { int x = 42 }");
    auto tokens1 = lexer1.tokenize();
    ris::Parser parser1(tokens1);
    auto program1 = parser1.parse();
    
    ASSERT_TRUE(parser1.has_error());
    
    // Test missing closing brace
    ris::Lexer lexer2("int main() { int x = 42;");
    auto tokens2 = lexer2.tokenize();
    ris::Parser parser2(tokens2);
    auto program2 = parser2.parse();
    
    ASSERT_TRUE(parser2.has_error());
    
    
    return 0;
}

int test_parser_switch_statement() {
    std::cout << "Running test_parser_switch_statement .........";
    
    ris::Lexer lexer(R"(
        int main() {
            int x = 1;
            switch (x) {
                case 1:
                    return 1;
                case 2:
                    return 2;
                default:
                    return 0;
            }
        }
    )");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(2, func->body->statements.size());
    
    
    return 0;
}

int test_parser_break_statement() {
    std::cout << "Running test_parser_break_statement .........";
    
    ris::Lexer lexer("int main() { while (true) { break; } return 0; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(2, func->body->statements.size());
    
    
    return 0;
}

int test_parser_continue_statement() {
    std::cout << "Running test_parser_continue_statement .........";
    
    ris::Lexer lexer("int main() { while (true) { continue; } return 0; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(2, func->body->statements.size());
    
    
    return 0;
}

int test_parser_list_literal() {
    std::cout << "Running test_parser_list_literal .........";
    
    ris::Lexer lexer("int main() { list<int> a = [1, 2, 3]; return 0; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(2, func->body->statements.size());
    
    
    return 0;
}

int test_parser_list_method_calls() {
    std::cout << "Running test_parser_list_method_calls .........";
    
    ris::Lexer lexer("int main() { list<int> a = [1, 2, 3]; a.push(4); a.pop(); a.size(); return 0; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(5, func->body->statements.size());
    
    
    return 0;
}

int test_parser_list_indexing() {
    std::cout << "Running test_parser_list_indexing .........";
    
    ris::Lexer lexer("int main() { list<int> a = [1, 2, 3]; int x = a[0]; return x; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(3, func->body->statements.size());
    
    
    return 0;
}

// Comprehensive parser tests for all AST node types and complex expressions

int test_parser_complex_expressions() {
    std::string code = "int main() { return (a + b) * (c - d) / (e % f); }";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(1, func->body->statements.size());
    
    auto* return_stmt = dynamic_cast<ris::ReturnStmt*>(func->body->statements[0].get());
    ASSERT_TRUE(return_stmt != nullptr);
    ASSERT_TRUE(return_stmt->value != nullptr);
    
    auto* binary_expr = dynamic_cast<ris::BinaryExpr*>(return_stmt->value.get());
    ASSERT_TRUE(binary_expr != nullptr);
    ASSERT_EQ(ris::TokenType::DIVIDE, binary_expr->op);
    
    return 0;
}

int test_parser_nested_control_flow() {
    std::string code = R"(
        int main() {
            if (x > 0) {
                while (y < 10) {
                    for (int i = 0; i < 5; i = i + 1) {
                        if (i % 2 == 0) {
                            break;
                        }
                    }
                }
            }
            return 0;
        }
    )";
    
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(2, func->body->statements.size()); // if + return
    
    auto* if_stmt = dynamic_cast<ris::IfStmt*>(func->body->statements[0].get());
    ASSERT_TRUE(if_stmt != nullptr);
    ASSERT_TRUE(if_stmt->then_branch != nullptr);
    
    auto* while_stmt = dynamic_cast<ris::WhileStmt*>(if_stmt->then_branch.get());
    ASSERT_TRUE(while_stmt != nullptr);
    ASSERT_TRUE(while_stmt->body != nullptr);
    
    auto* for_stmt = dynamic_cast<ris::ForStmt*>(while_stmt->body.get());
    ASSERT_TRUE(for_stmt != nullptr);
    ASSERT_TRUE(for_stmt->body != nullptr);
    
    auto* nested_if = dynamic_cast<ris::IfStmt*>(for_stmt->body.get());
    ASSERT_TRUE(nested_if != nullptr);
    ASSERT_TRUE(nested_if->then_branch != nullptr);
    
    auto* break_stmt = dynamic_cast<ris::BreakStmt*>(nested_if->then_branch.get());
    ASSERT_TRUE(break_stmt != nullptr);
    
    return 0;
}

int test_parser_switch_statement_complex() {
    std::string code = R"(
        int main() {
            switch (x) {
                case 1:
                    return 10;
                case 2:
                case 3:
                    return 20;
                default:
                    return 0;
            }
        }
    )";
    
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(1, func->body->statements.size());
    
    auto* switch_stmt = dynamic_cast<ris::SwitchStmt*>(func->body->statements[0].get());
    ASSERT_TRUE(switch_stmt != nullptr);
    ASSERT_EQ(4, switch_stmt->cases.size()); // 3 cases + default
    
    // Check first case
    auto* case1 = switch_stmt->cases[0].get();
    ASSERT_TRUE(case1->value != nullptr);
    auto* case1_literal = dynamic_cast<ris::LiteralExpr*>(case1->value.get());
    ASSERT_TRUE(case1_literal != nullptr);
    ASSERT_EQ("1", case1_literal->value);
    ASSERT_EQ(1, case1->statements.size());
    
    // Check default case
    auto* default_case = switch_stmt->cases[3].get();
    ASSERT_TRUE(default_case->value == nullptr); // default case has no value
    ASSERT_EQ(1, default_case->statements.size());
    
    return 0;
}

int test_parser_list_operations_complex() {
    std::string code = R"(
        int main() {
            list<int> numbers = [1, 2, 3, 4, 5];
            numbers.push(6);
            int first = numbers[0];
            int size = numbers.size();
            return first + size;
        }
    )";
    
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(5, func->body->statements.size()); // var decl + 3 statements + return
    
    // Check list literal
    auto* var_decl = dynamic_cast<ris::VarDecl*>(func->body->statements[0].get());
    ASSERT_TRUE(var_decl != nullptr);
    ASSERT_EQ("numbers", var_decl->name);
    ASSERT_EQ("list<int>", var_decl->type);
    ASSERT_TRUE(var_decl->initializer != nullptr);
    
    auto* list_literal = dynamic_cast<ris::ListLiteralExpr*>(var_decl->initializer.get());
    ASSERT_TRUE(list_literal != nullptr);
    ASSERT_EQ(5, list_literal->elements.size());
    
    // Check list method call
    auto* expr_stmt = dynamic_cast<ris::ExprStmt*>(func->body->statements[1].get());
    ASSERT_TRUE(expr_stmt != nullptr);
    auto* method_call = dynamic_cast<ris::ListMethodCallExpr*>(expr_stmt->expression.get());
    ASSERT_TRUE(method_call != nullptr);
    ASSERT_EQ("push", method_call->method_name);
    ASSERT_EQ(1, method_call->arguments.size());
    
    // Check list indexing
    auto* var_decl2 = dynamic_cast<ris::VarDecl*>(func->body->statements[2].get());
    ASSERT_TRUE(var_decl2 != nullptr);
    auto* index_expr = dynamic_cast<ris::ListIndexExpr*>(var_decl2->initializer.get());
    ASSERT_TRUE(index_expr != nullptr);
    ASSERT_TRUE(index_expr->list != nullptr);
    ASSERT_TRUE(index_expr->index != nullptr);
    
    return 0;
}

int test_parser_function_overloading() {
    std::string code = R"(
        int add(int a, int b) { return a + b; }
        float add(float a, float b) { return a + b; }
        int main() { return add(1, 2) + add(1.5, 2.5); }
    )";
    
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(3, program->functions.size());
    
    // Check first add function
    auto& func1 = program->functions[0];
    ASSERT_EQ("add", func1->name);
    ASSERT_EQ("int", func1->return_type);
    ASSERT_EQ(2, func1->parameters.size());
    ASSERT_EQ("int", func1->parameters[0].first);
    ASSERT_EQ("a", func1->parameters[0].second);
    ASSERT_EQ("int", func1->parameters[1].first);
    ASSERT_EQ("b", func1->parameters[1].second);
    
    // Check second add function
    auto& func2 = program->functions[1];
    ASSERT_EQ("add", func2->name);
    ASSERT_EQ("float", func2->return_type);
    ASSERT_EQ(2, func2->parameters.size());
    ASSERT_EQ("float", func2->parameters[0].first);
    ASSERT_EQ("a", func2->parameters[0].second);
    ASSERT_EQ("float", func2->parameters[1].first);
    ASSERT_EQ("b", func2->parameters[1].second);
    
    return 0;
}

int test_parser_global_variables() {
    std::string code = R"(
        int global_int = 42;
        float global_float = 3.14;
        bool global_bool = true;
        string global_string = "hello";
        
        int main() {
            return global_int;
        }
    )";
    
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(4, program->globals.size());
    ASSERT_EQ(1, program->functions.size());
    
    // Check global variables
    auto& global1 = program->globals[0];
    ASSERT_EQ("global_int", global1->name);
    ASSERT_EQ("int", global1->type);
    ASSERT_TRUE(global1->initializer != nullptr);
    
    auto& global2 = program->globals[1];
    ASSERT_EQ("global_float", global2->name);
    ASSERT_EQ("float", global2->type);
    ASSERT_TRUE(global2->initializer != nullptr);
    
    auto& global3 = program->globals[2];
    ASSERT_EQ("global_bool", global3->name);
    ASSERT_EQ("bool", global3->type);
    ASSERT_TRUE(global3->initializer != nullptr);
    
    auto& global4 = program->globals[3];
    ASSERT_EQ("global_string", global4->name);
    ASSERT_EQ("string", global4->type);
    ASSERT_TRUE(global4->initializer != nullptr);
    
    return 0;
}

int test_parser_increment_operators() {
    std::string code = R"(
        int main() {
            int x = 5;
            ++x;
            x++;
            return x;
        }
    )";
    
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(4, func->body->statements.size()); // var decl + 2 increments + return
    
    // Check pre-increment
    auto* expr_stmt1 = dynamic_cast<ris::ExprStmt*>(func->body->statements[1].get());
    ASSERT_TRUE(expr_stmt1 != nullptr);
    auto* pre_inc = dynamic_cast<ris::PreIncrementExpr*>(expr_stmt1->expression.get());
    ASSERT_TRUE(pre_inc != nullptr);
    ASSERT_TRUE(pre_inc->operand != nullptr);
    
    // Check post-increment
    auto* expr_stmt2 = dynamic_cast<ris::ExprStmt*>(func->body->statements[2].get());
    ASSERT_TRUE(expr_stmt2 != nullptr);
    auto* post_inc = dynamic_cast<ris::PostIncrementExpr*>(expr_stmt2->expression.get());
    ASSERT_TRUE(post_inc != nullptr);
    ASSERT_TRUE(post_inc->operand != nullptr);
    
    return 0;
}

int test_parser_unary_operators() {
    std::string code = R"(
        int main() {
            int x = 5;
            int y = -x;
            bool z = !true;
            return y + z;
        }
    )";
    
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ("main", func->name);
    ASSERT_EQ(4, func->body->statements.size()); // 3 var decls + return
    
    // Check unary minus
    auto* var_decl2 = dynamic_cast<ris::VarDecl*>(func->body->statements[1].get());
    ASSERT_TRUE(var_decl2 != nullptr);
    auto* unary_minus = dynamic_cast<ris::UnaryExpr*>(var_decl2->initializer.get());
    ASSERT_TRUE(unary_minus != nullptr);
    ASSERT_EQ(ris::TokenType::MINUS, unary_minus->op);
    ASSERT_TRUE(unary_minus->operand != nullptr);
    
    // Check unary not
    auto* var_decl3 = dynamic_cast<ris::VarDecl*>(func->body->statements[2].get());
    ASSERT_TRUE(var_decl3 != nullptr);
    auto* unary_not = dynamic_cast<ris::UnaryExpr*>(var_decl3->initializer.get());
    ASSERT_TRUE(unary_not != nullptr);
    ASSERT_EQ(ris::TokenType::NOT, unary_not->op);
    ASSERT_TRUE(unary_not->operand != nullptr);
    
    return 0;
}

int test_parser_error_handling_comprehensive() {
    // Test missing semicolon
    ris::Lexer lexer1("int main() { int x = 5 }");
    auto tokens1 = lexer1.tokenize();
    ris::Parser parser1(tokens1);
    auto program1 = parser1.parse();
    ASSERT_TRUE(parser1.has_error());
    
    // Test missing closing brace
    ris::Lexer lexer2("int main() { int x = 5;");
    auto tokens2 = lexer2.tokenize();
    ris::Parser parser2(tokens2);
    auto program2 = parser2.parse();
    ASSERT_TRUE(parser2.has_error());
    
    // Test invalid expression
    ris::Lexer lexer3("int main() { int x = +; }");
    auto tokens3 = lexer3.tokenize();
    ris::Parser parser3(tokens3);
    auto program3 = parser3.parse();
    ASSERT_TRUE(parser3.has_error());
    
    return 0;
}

int test_parser_edge_cases() {
    // Test empty function
    ris::Lexer lexer1("void empty() { }");
    auto tokens1 = lexer1.tokenize();
    ris::Parser parser1(tokens1);
    auto program1 = parser1.parse();
    ASSERT_FALSE(parser1.has_error());
    ASSERT_TRUE(program1 != nullptr);
    ASSERT_EQ(1, program1->functions.size());
    
    auto& func1 = program1->functions[0];
    ASSERT_EQ("empty", func1->name);
    ASSERT_EQ("void", func1->return_type);
    ASSERT_EQ(0, func1->body->statements.size());
    
    // Test function with no parameters
    ris::Lexer lexer2("int no_params() { return 42; }");
    auto tokens2 = lexer2.tokenize();
    ris::Parser parser2(tokens2);
    auto program2 = parser2.parse();
    ASSERT_FALSE(parser2.has_error());
    ASSERT_TRUE(program2 != nullptr);
    ASSERT_EQ(1, program2->functions.size());
    
    auto& func2 = program2->functions[0];
    ASSERT_EQ("no_params", func2->name);
    ASSERT_EQ(0, func2->parameters.size());
    
    return 0;
}

// Test functions are defined above, main() is in test_runner.cpp
