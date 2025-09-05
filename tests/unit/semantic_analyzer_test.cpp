#include "semantic_analyzer.h"
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

int test_semantic_valid_program() {
    std::cout << "Running test_semantic_valid_program..." << std::endl;
    
    ris::Lexer lexer(R"(
        int global_var = 42;
        
        int add(int a, int b) {
            return a + b;
        }
        
        int main() {
            int x = 5;
            int y = add(x, global_var);
            return y;
        }
    )");
    
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_TRUE(result);
    ASSERT_FALSE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_valid_program passed" << std::endl;
    return 0;
}

int test_semantic_undefined_variable() {
    std::cout << "Running test_semantic_undefined_variable..." << std::endl;
    
    ris::Lexer lexer("int main() { int x = undefined_var; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_FALSE(result);
    ASSERT_TRUE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_undefined_variable passed" << std::endl;
    return 0;
}

int test_semantic_duplicate_variable() {
    std::cout << "Running test_semantic_duplicate_variable..." << std::endl;
    
    ris::Lexer lexer("int main() { int x = 5; int x = 10; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_FALSE(result);
    ASSERT_TRUE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_duplicate_variable passed" << std::endl;
    return 0;
}

int test_semantic_type_mismatch() {
    std::cout << "Running test_semantic_type_mismatch..." << std::endl;
    
    ris::Lexer lexer("int main() { int x = true; }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_FALSE(result);
    ASSERT_TRUE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_type_mismatch passed" << std::endl;
    return 0;
}

int test_semantic_arithmetic_operations() {
    std::cout << "Running test_semantic_arithmetic_operations..." << std::endl;
    
    ris::Lexer lexer(R"(
        int main() {
            int a = 5;
            int b = 10;
            int c = a + b;
            int d = a - b;
            int e = a * b;
            int f = a / b;
            return c;
        }
    )");
    
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_TRUE(result);
    ASSERT_FALSE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_arithmetic_operations passed" << std::endl;
    return 0;
}

int test_semantic_boolean_operations() {
    std::cout << "Running test_semantic_boolean_operations..." << std::endl;
    
    ris::Lexer lexer(R"(
        int main() {
            bool a = true;
            bool b = false;
            bool c = a && b;
            bool d = a || b;
            bool e = !a;
            return 0;
        }
    )");
    
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_TRUE(result);
    ASSERT_FALSE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_boolean_operations passed" << std::endl;
    return 0;
}

int test_semantic_comparison_operations() {
    std::cout << "Running test_semantic_comparison_operations..." << std::endl;
    
    ris::Lexer lexer(R"(
        int main() {
            int a = 5;
            int b = 10;
            bool c = a < b;
            bool d = a > b;
            bool e = a == b;
            bool f = a != b;
            return 0;
        }
    )");
    
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_TRUE(result);
    ASSERT_FALSE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_comparison_operations passed" << std::endl;
    return 0;
}

int test_semantic_function_calls() {
    std::cout << "Running test_semantic_function_calls..." << std::endl;
    
    ris::Lexer lexer(R"(
        int add(int a, int b) {
            return a + b;
        }
        
        int main() {
            int x = add(5, 10);
            int y = add(x, 20);
            return y;
        }
    )");
    
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_TRUE(result);
    ASSERT_FALSE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_function_calls passed" << std::endl;
    return 0;
}

int test_semantic_wrong_argument_count() {
    std::cout << "Running test_semantic_wrong_argument_count..." << std::endl;
    
    ris::Lexer lexer(R"(
        int add(int a, int b) {
            return a + b;
        }
        
        int main() {
            int x = add(5);  // Wrong number of arguments
            return x;
        }
    )");
    
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_FALSE(result);
    ASSERT_TRUE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_wrong_argument_count passed" << std::endl;
    return 0;
}

int test_semantic_undefined_function() {
    std::cout << "Running test_semantic_undefined_function..." << std::endl;
    
    ris::Lexer lexer("int main() { int x = undefined_func(5); }");
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_FALSE(result);
    ASSERT_TRUE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_undefined_function passed" << std::endl;
    return 0;
}

int test_semantic_control_flow() {
    std::cout << "Running test_semantic_control_flow..." << std::endl;
    
    ris::Lexer lexer(R"(
        int main() {
            int x = 5;
            if (x > 0) {
                x = x + 1;
            } else {
                x = x - 1;
            }
            
            while (x > 0) {
                x = x - 1;
            }
            
            for (int i = 0; i < 10; i = i + 1) {
                x = x + i;
            }
            
            return x;
        }
    )");
    
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_TRUE(result);
    ASSERT_FALSE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_control_flow passed" << std::endl;
    return 0;
}

int test_semantic_array_operations() {
    std::cout << "Running test_semantic_array_operations..." << std::endl;
    
    ris::Lexer lexer(R"(
        int main() {
            int arr[10];
            int x = arr[0];
            arr[1] = 42;
            return x;
        }
    )");
    
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_TRUE(result);
    ASSERT_FALSE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_array_operations passed" << std::endl;
    return 0;
}

int test_semantic_scope_handling() {
    std::cout << "Running test_semantic_scope_handling..." << std::endl;
    
    ris::Lexer lexer(R"(
        int main() {
            int x = 5;
            {
                int y = 10;
                int z = x + y;  // Should work - x is in outer scope
            }
            int w = y;  // Should fail - y is not in scope
            return x;
        }
    )");
    
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_FALSE(result);
    ASSERT_TRUE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_scope_handling passed" << std::endl;
    return 0;
}

int test_semantic_implicit_conversions() {
    std::cout << "Running test_semantic_implicit_conversions..." << std::endl;
    
    ris::Lexer lexer(R"(
        int main() {
            int x = 5;
            float y = x;  // int to float conversion should be allowed
            char c = 'a';
            int z = c;    // char to int conversion should be allowed
            return 0;
        }
    )");
    
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    ASSERT_FALSE(parser.has_error());
    ASSERT_TRUE(program != nullptr);
    
    ris::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(std::move(program));
    
    ASSERT_TRUE(result);
    ASSERT_FALSE(analyzer.has_error());
    
    std::cout << "✓ test_semantic_implicit_conversions passed" << std::endl;
    return 0;
}

// Test functions are defined above, main() is in test_runner.cpp
