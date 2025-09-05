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
    std::cout << "Running test_parser_basic_function..." << std::endl;
    
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
    
    std::cout << "✓ test_parser_basic_function passed" << std::endl;
    return 0;
}

int test_parser_function_with_parameters() {
    std::cout << "Running test_parser_function_with_parameters..." << std::endl;
    
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
    
    std::cout << "✓ test_parser_function_with_parameters passed" << std::endl;
    return 0;
}

int test_parser_variable_declaration() {
    std::cout << "Running test_parser_variable_declaration..." << std::endl;
    
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
    
    std::cout << "✓ test_parser_variable_declaration passed" << std::endl;
    return 0;
}

int test_parser_array_declaration() {
    std::cout << "Running test_parser_array_declaration..." << std::endl;
    
    ris::Lexer lexer("int arr[10]; int dynamic[];");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(2, program->globals.size());
    
    auto& arr = program->globals[0];
    ASSERT_EQ("arr", arr->name);
    ASSERT_EQ("int", arr->type);
    ASSERT_TRUE(arr->is_array);
    ASSERT_EQ(10, arr->array_size);
    
    auto& dynamic = program->globals[1];
    ASSERT_EQ("dynamic", dynamic->name);
    ASSERT_EQ("int", dynamic->type);
    ASSERT_TRUE(dynamic->is_array);
    ASSERT_EQ(-1, dynamic->array_size);
    
    std::cout << "✓ test_parser_array_declaration passed" << std::endl;
    return 0;
}

int test_parser_arithmetic_expressions() {
    std::cout << "Running test_parser_arithmetic_expressions..." << std::endl;
    
    ris::Lexer lexer("int main() { int x = 1 + 2 * 3; int y = (4 - 2) / 2; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(2, func->body->statements.size());
    
    std::cout << "✓ test_parser_arithmetic_expressions passed" << std::endl;
    return 0;
}

int test_parser_logical_expressions() {
    std::cout << "Running test_parser_logical_expressions..." << std::endl;
    
    ris::Lexer lexer("int main() { bool x = true && false; bool y = a > b || c < d; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(2, func->body->statements.size());
    
    std::cout << "✓ test_parser_logical_expressions passed" << std::endl;
    return 0;
}

int test_parser_if_statement() {
    std::cout << "Running test_parser_if_statement..." << std::endl;
    
    ris::Lexer lexer("int main() { if (x > 0) { return 1; } else { return 0; } }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(1, func->body->statements.size());
    
    std::cout << "✓ test_parser_if_statement passed" << std::endl;
    return 0;
}

int test_parser_while_statement() {
    std::cout << "Running test_parser_while_statement..." << std::endl;
    
    ris::Lexer lexer("int main() { while (x > 0) { x = x - 1; } }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(1, func->body->statements.size());
    
    std::cout << "✓ test_parser_while_statement passed" << std::endl;
    return 0;
}

int test_parser_for_statement() {
    std::cout << "Running test_parser_for_statement..." << std::endl;
    
    ris::Lexer lexer("int main() { for (int i = 0; i < 10; i = i + 1) { x = x + i; } }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(1, func->body->statements.size());
    
    std::cout << "✓ test_parser_for_statement passed" << std::endl;
    return 0;
}

int test_parser_return_statement() {
    std::cout << "Running test_parser_return_statement..." << std::endl;
    
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
    
    std::cout << "✓ test_parser_return_statement passed" << std::endl;
    return 0;
}

int test_parser_function_call() {
    std::cout << "Running test_parser_function_call..." << std::endl;
    
    ris::Lexer lexer("int main() { int x = add(1, 2); int y = func(); }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(2, func->body->statements.size());
    
    std::cout << "✓ test_parser_function_call passed" << std::endl;
    return 0;
}

int test_parser_array_access() {
    std::cout << "Running test_parser_array_access..." << std::endl;
    
    ris::Lexer lexer("int main() { int x = arr[0]; int y = arr[i + 1]; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    ASSERT_EQ(1, program->functions.size());
    
    auto& func = program->functions[0];
    ASSERT_EQ(2, func->body->statements.size());
    
    std::cout << "✓ test_parser_array_access passed" << std::endl;
    return 0;
}

int test_parser_complex_program() {
    std::cout << "Running test_parser_complex_program..." << std::endl;
    
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
    
    std::cout << "✓ test_parser_complex_program passed" << std::endl;
    return 0;
}

int test_parser_error_handling() {
    std::cout << "Running test_parser_error_handling..." << std::endl;
    
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
    
    std::cout << "✓ test_parser_error_handling passed" << std::endl;
    return 0;
}

// Test functions are defined above, main() is in test_runner.cpp
