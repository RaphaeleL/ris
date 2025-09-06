#include "parser.h"
#include "lexer.h"
#include <iostream>
#include <cassert>

// Simple test framework
#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            std::cerr << "FAIL: " << #expected << " != " << #actual << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while(0)

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << "FAIL: " << #condition << " is false at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            std::cerr << "FAIL: " << #condition << " is true at " << __FILE__ << ":" << __LINE__ << std::endl; \
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
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
    
    std::cout << " OK" << std::endl;
    return 0;
}

// Test functions are defined above, main() is in test_runner.cpp
