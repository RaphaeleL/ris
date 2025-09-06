#include "parser.h"
#include "lexer.h"
#include <iostream>
#include <cassert>
#include <vector>

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

// Helper function to parse code and return AST
std::unique_ptr<ris::Program> parse_code(const std::string& code) {
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    return parser.parse();
}

int test_parser_complex_expressions() {
    std::cout << "Running test_parser_complex_expressions .........";
    
    std::string code = "int main() { return (a + b) * (c - d) / (e % f); }";
    auto program = parse_code(code);
    
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto* func = program->functions[0].get();
    ASSERT_EQ("main", func->name);
    ASSERT_EQ("int", func->return_type);
    
    auto* return_stmt = dynamic_cast<ris::ReturnStmt*>(func->body->statements[0].get());
    ASSERT_TRUE(return_stmt != nullptr);
    
    auto* binary_expr = dynamic_cast<ris::BinaryExpr*>(return_stmt->value.get());
    ASSERT_TRUE(binary_expr != nullptr);
    ASSERT_EQ(ris::TokenType::DIVIDE, binary_expr->op);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_parser_nested_control_flow() {
    std::cout << "Running test_parser_nested_control_flow .........";
    
    std::string code = R"(
        int main() {
            if (x > 0) {
                while (y < 10) {
                    for (int i = 0; i < 5; i = i + 1) {
                        if (i % 2 == 0) {
                            break;
                        } else {
                            continue;
                        }
                    }
                }
            }
            return 0;
        }
    )";
    
    auto program = parse_code(code);
    ASSERT_TRUE(program != nullptr);
    
    auto* func = program->functions[0].get();
    auto* if_stmt = dynamic_cast<ris::IfStmt*>(func->body->statements[0].get());
    ASSERT_TRUE(if_stmt != nullptr);
    
    auto* while_stmt = dynamic_cast<ris::WhileStmt*>(if_stmt->then_branch.get());
    ASSERT_TRUE(while_stmt != nullptr);
    
    auto* for_stmt = dynamic_cast<ris::ForStmt*>(while_stmt->body.get());
    ASSERT_TRUE(for_stmt != nullptr);
    
    auto* nested_if = dynamic_cast<ris::IfStmt*>(for_stmt->body.get());
    ASSERT_TRUE(nested_if != nullptr);
    
    auto* break_stmt = dynamic_cast<ris::BreakStmt*>(nested_if->then_branch.get());
    ASSERT_TRUE(break_stmt != nullptr);
    
    auto* continue_stmt = dynamic_cast<ris::ContinueStmt*>(nested_if->else_branch.get());
    ASSERT_TRUE(continue_stmt != nullptr);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_parser_switch_statement_comprehensive() {
    std::cout << "Running test_parser_switch_statement_comprehensive .........";
    
    std::string code = R"(
        int main() {
            switch (x) {
                case 1:
                    return 1;
                case 2:
                case 3:
                    return 2;
                default:
                    return 0;
            }
        }
    )";
    
    auto program = parse_code(code);
    ASSERT_TRUE(program != nullptr);
    
    auto* func = program->functions[0].get();
    auto* switch_stmt = dynamic_cast<ris::SwitchStmt*>(func->body->statements[0].get());
    ASSERT_TRUE(switch_stmt != nullptr);
    
    ASSERT_EQ(4, switch_stmt->cases.size());
    
    // Check first case
    auto* case1 = switch_stmt->cases[0].get();
    ASSERT_TRUE(case1->value != nullptr);
    auto* literal1 = dynamic_cast<ris::LiteralExpr*>(case1->value.get());
    ASSERT_TRUE(literal1 != nullptr);
    ASSERT_EQ("1", literal1->value);
    
    // Check default case
    auto* default_case = switch_stmt->cases[3].get();
    ASSERT_TRUE(default_case->value == nullptr); // default case has no value
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_parser_list_operations_comprehensive() {
    std::cout << "Running test_parser_list_operations_comprehensive .........";
    
    std::string code = R"(
        int main() {
            list<int> numbers = [1, 2, 3, 4, 5];
            int first = numbers[0];
            numbers.push(6);
            int size = numbers.size();
            int popped = numbers.pop();
            return first + size + popped;
        }
    )";
    
    auto program = parse_code(code);
    ASSERT_TRUE(program != nullptr);
    
    auto* func = program->functions[0].get();
    
    // Check list literal
    auto* var_decl = dynamic_cast<ris::VarDecl*>(func->body->statements[0].get());
    ASSERT_TRUE(var_decl != nullptr);
    ASSERT_EQ("numbers", var_decl->name);
    ASSERT_EQ("list<int>", var_decl->type);
    
    auto* list_literal = dynamic_cast<ris::ListLiteralExpr*>(var_decl->initializer.get());
    ASSERT_TRUE(list_literal != nullptr);
    ASSERT_EQ(5, list_literal->elements.size());
    
    // Check list indexing
    auto* var_decl2 = dynamic_cast<ris::VarDecl*>(func->body->statements[1].get());
    auto* list_index = dynamic_cast<ris::ListIndexExpr*>(var_decl2->initializer.get());
    ASSERT_TRUE(list_index != nullptr);
    
    // Check list method call (push)
    auto* expr_stmt = dynamic_cast<ris::ExprStmt*>(func->body->statements[2].get());
    auto* method_call = dynamic_cast<ris::ListMethodCallExpr*>(expr_stmt->expression.get());
    ASSERT_TRUE(method_call != nullptr);
    ASSERT_EQ("push", method_call->method_name);
    ASSERT_EQ(1, method_call->arguments.size());
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_parser_function_overloading_simulation() {
    std::cout << "Running test_parser_function_overloading_simulation .........";
    
    std::string code = R"(
        int add(int a, int b) {
            return a + b;
        }
        
        float add(float a, float b) {
            return a + b;
        }
        
        int main() {
            int result1 = add(1, 2);
            float result2 = add(1.5, 2.5);
            return result1;
        }
    )";
    
    auto program = parse_code(code);
    ASSERT_TRUE(program != nullptr);
    
    ASSERT_EQ(3, program->functions.size());
    
    // Check first add function
    auto* add1 = program->functions[0].get();
    ASSERT_EQ("add", add1->name);
    ASSERT_EQ("int", add1->return_type);
    ASSERT_EQ(2, add1->parameters.size());
    ASSERT_EQ("int", add1->parameters[0].first);
    ASSERT_EQ("a", add1->parameters[0].second);
    
    // Check second add function
    auto* add2 = program->functions[1].get();
    ASSERT_EQ("add", add2->name);
    ASSERT_EQ("float", add2->return_type);
    ASSERT_EQ(2, add2->parameters.size());
    ASSERT_EQ("float", add2->parameters[0].first);
    ASSERT_EQ("a", add2->parameters[0].second);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_parser_global_variables() {
    std::cout << "Running test_parser_global_variables .........";
    
    std::string code = R"(
        int global_int = 42;
        float global_float = 3.14;
        bool global_bool = true;
        string global_string = "hello";
        list<int> global_list = [1, 2, 3];
        
        int main() {
            return global_int;
        }
    )";
    
    auto program = parse_code(code);
    ASSERT_TRUE(program != nullptr);
    
    ASSERT_EQ(5, program->globals.size());
    ASSERT_EQ(1, program->functions.size());
    
    // Check global variables
    auto* global1 = program->globals[0].get();
    ASSERT_EQ("global_int", global1->name);
    ASSERT_EQ("int", global1->type);
    
    auto* global2 = program->globals[1].get();
    ASSERT_EQ("global_float", global2->name);
    ASSERT_EQ("float", global2->type);
    
    auto* global3 = program->globals[2].get();
    ASSERT_EQ("global_bool", global3->name);
    ASSERT_EQ("bool", global3->type);
    
    auto* global4 = program->globals[3].get();
    ASSERT_EQ("global_string", global4->name);
    ASSERT_EQ("string", global4->type);
    
    auto* global5 = program->globals[4].get();
    ASSERT_EQ("global_list", global5->name);
    ASSERT_EQ("list<int>", global5->type);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_parser_increment_operators() {
    std::cout << "Running test_parser_increment_operators .........";
    
    std::string code = R"(
        int main() {
            int x = 5;
            ++x;
            x++;
            return x;
        }
    )";
    
    auto program = parse_code(code);
    ASSERT_TRUE(program != nullptr);
    
    auto* func = program->functions[0].get();
    
    // Check pre-increment
    auto* expr_stmt1 = dynamic_cast<ris::ExprStmt*>(func->body->statements[1].get());
    auto* pre_inc = dynamic_cast<ris::PreIncrementExpr*>(expr_stmt1->expression.get());
    ASSERT_TRUE(pre_inc != nullptr);
    
    // Check post-increment
    auto* expr_stmt2 = dynamic_cast<ris::ExprStmt*>(func->body->statements[2].get());
    auto* post_inc = dynamic_cast<ris::PostIncrementExpr*>(expr_stmt2->expression.get());
    ASSERT_TRUE(post_inc != nullptr);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_parser_complex_arithmetic() {
    std::cout << "Running test_parser_complex_arithmetic .........";
    
    std::string code = R"(
        int main() {
            int a = 1 + 2 * 3 - 4 / 2;
            int b = (1 + 2) * (3 - 4) / 2;
            int c = a % b;
            bool d = a > b && b < c || c == a;
            return a + b + c;
        }
    )";
    
    auto program = parse_code(code);
    ASSERT_TRUE(program != nullptr);
    
    auto* func = program->functions[0].get();
    
    // Check complex arithmetic expression
    auto* var_decl1 = dynamic_cast<ris::VarDecl*>(func->body->statements[0].get());
    auto* binary_expr = dynamic_cast<ris::BinaryExpr*>(var_decl1->initializer.get());
    ASSERT_TRUE(binary_expr != nullptr);
    ASSERT_EQ(ris::TokenType::MINUS, binary_expr->op);
    
    // Check boolean expression
    auto* var_decl4 = dynamic_cast<ris::VarDecl*>(func->body->statements[3].get());
    auto* bool_expr = dynamic_cast<ris::BinaryExpr*>(var_decl4->initializer.get());
    ASSERT_TRUE(bool_expr != nullptr);
    ASSERT_EQ(ris::TokenType::OR, bool_expr->op);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_parser_error_recovery() {
    std::cout << "Running test_parser_error_recovery .........";
    
    // Test missing semicolon
    std::string code1 = "int main() { int x = 5 return x; }";
    auto program1 = parse_code(code1);
    // Should handle error gracefully
    
    // Test missing closing brace
    std::string code2 = "int main() { int x = 5;";
    auto program2 = parse_code(code2);
    // Should handle error gracefully
    
    // Test missing parenthesis
    std::string code3 = "int main( { int x = 5; return x; }";
    auto program3 = parse_code(code3);
    // Should handle error gracefully
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_parser_edge_cases() {
    std::cout << "Running test_parser_edge_cases .........";
    
    // Empty function
    std::string code1 = "void empty() { }";
    auto program1 = parse_code(code1);
    ASSERT_TRUE(program1 != nullptr);
    
    // Function with no parameters
    std::string code2 = "int no_params() { return 42; }";
    auto program2 = parse_code(code2);
    ASSERT_TRUE(program2 != nullptr);
    
    // Function with many parameters
    std::string code3 = "int many_params(int a, int b, int c, int d, int e) { return a + b + c + d + e; }";
    auto program3 = parse_code(code3);
    ASSERT_TRUE(program3 != nullptr);
    ASSERT_EQ(5, program3->functions[0]->parameters.size());
    
    // Empty list literal
    std::string code4 = "int main() { list<int> empty = []; return 0; }";
    auto program4 = parse_code(code4);
    ASSERT_TRUE(program4 != nullptr);
    
    std::cout << " OK" << std::endl;
    return 0;
}
