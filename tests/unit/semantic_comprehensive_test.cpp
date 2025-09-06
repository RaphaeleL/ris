#include "semantic_analyzer.h"
#include "parser.h"
#include "lexer.h"
#include <iostream>
#include <cassert>

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

// Helper function to analyze code
bool analyze_code(const std::string& code) {
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    if (parser.has_error() || !program) {
        return false;
    }
    
    ris::SemanticAnalyzer analyzer;
    return analyzer.analyze(*program);
}

int test_semantic_type_coercion() {
    std::cout << "Running test_semantic_type_coercion .........";
    
    // Test implicit int to float conversion
    std::string code1 = R"(
        float func() {
            int x = 5;
            float y = x;  // Should allow implicit conversion
            return y;
        }
    )";
    ASSERT_TRUE(analyze_code(code1));
    
    // Test explicit type conversion
    std::string code2 = R"(
        int func() {
            float x = 3.14;
            int y = (int)x;  // Should allow explicit cast
            return y;
        }
    )";
    ASSERT_TRUE(analyze_code(code2));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_semantic_scope_nesting() {
    std::cout << "Running test_semantic_scope_nesting .........";
    
    std::string code = R"(
        int global = 10;
        
        int outer_func() {
            int outer = 20;
            
            if (true) {
                int inner = 30;
                return global + outer + inner;
            }
            
            return global + outer;
        }
        
        int main() {
            int local = 40;
            return outer_func() + local;
        }
    )";
    
    ASSERT_TRUE(analyze_code(code));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_semantic_variable_shadowing() {
    std::cout << "Running test_semantic_variable_shadowing .........";
    
    std::string code = R"(
        int x = 10;
        
        int func() {
            int x = 20;  // Should shadow global x
            if (true) {
                int x = 30;  // Should shadow local x
                return x;  // Should return 30
            }
            return x;  // Should return 20
        }
        
        int main() {
            return func() + x;  // Should return 30 + 10 = 40
        }
    )";
    
    ASSERT_TRUE(analyze_code(code));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_semantic_function_signature_matching() {
    std::cout << "Running test_semantic_function_signature_matching .........";
    
    std::string code = R"(
        int add(int a, int b) {
            return a + b;
        }
        
        float add_float(float a, float b) {
            return a + b;
        }
        
        int main() {
            int result1 = add(1, 2);
            float result2 = add_float(1.5, 2.5);
            return result1;
        }
    )";
    
    ASSERT_TRUE(analyze_code(code));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_semantic_list_type_checking() {
    std::cout << "Running test_semantic_list_type_checking .........";
    
    std::string code = R"(
        int main() {
            list<int> int_list = [1, 2, 3];
            list<float> float_list = [1.5, 2.5, 3.5];
            list<string> string_list = ["hello", "world"];
            
            int first_int = int_list[0];
            float first_float = float_list[0];
            string first_string = string_list[0];
            
            int_list.push(4);
            float_list.push(4.5);
            string_list.push("test");
            
            return first_int;
        }
    )";
    
    ASSERT_TRUE(analyze_code(code));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_semantic_control_flow_type_checking() {
    std::cout << "Running test_semantic_control_flow_type_checking .........";
    
    std::string code = R"(
        int main() {
            int x = 5;
            float y = 3.14;
            bool flag = true;
            
            if (x > 0) {
                return 1;
            } else if (y < 10.0) {
                return 2;
            } else if (flag) {
                return 3;
            }
            
            while (x > 0) {
                x = x - 1;
            }
            
            for (int i = 0; i < 10; i = i + 1) {
                if (i % 2 == 0) {
                    continue;
                }
                if (i > 5) {
                    break;
                }
            }
            
            return 0;
        }
    )";
    
    ASSERT_TRUE(analyze_code(code));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_semantic_switch_type_checking() {
    std::cout << "Running test_semantic_switch_type_checking .........";
    
    std::string code = R"(
        int main() {
            int x = 5;
            
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
    
    ASSERT_TRUE(analyze_code(code));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_semantic_arithmetic_type_promotion() {
    std::cout << "Running test_semantic_arithmetic_type_promotion .........";
    
    std::string code = R"(
        int main() {
            int a = 5;
            float b = 3.14;
            
            // Mixed arithmetic should promote to float
            float result1 = a + b;
            float result2 = a * b;
            float result3 = a / b;
            
            // Boolean operations
            bool flag1 = a > 0;
            bool flag2 = b < 10.0;
            bool flag3 = flag1 && flag2;
            
            return a;
        }
    )";
    
    ASSERT_TRUE(analyze_code(code));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_semantic_error_detection() {
    std::cout << "Running test_semantic_error_detection .........";
    
    // Test undefined variable
    std::string code1 = R"(
        int main() {
            return undefined_var;  // Should fail
        }
    )";
    ASSERT_FALSE(analyze_code(code1));
    
    // Test type mismatch in assignment
    std::string code2 = R"(
        int main() {
            int x = "hello";  // Should fail
            return x;
        }
    )";
    ASSERT_FALSE(analyze_code(code2));
    
    // Test wrong argument count
    std::string code3 = R"(
        int add(int a, int b) {
            return a + b;
        }
        
        int main() {
            return add(1, 2, 3);  // Should fail
        }
    )";
    ASSERT_FALSE(analyze_code(code3));
    
    // Test wrong argument types
    std::string code4 = R"(
        int add(int a, int b) {
            return a + b;
        }
        
        int main() {
            return add(1, "hello");  // Should fail
        }
    )";
    ASSERT_FALSE(analyze_code(code4));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_semantic_return_type_checking() {
    std::cout << "Running test_semantic_return_type_checking .........";
    
    // Test correct return types
    std::string code1 = R"(
        int int_func() {
            return 42;
        }
        
        float float_func() {
            return 3.14;
        }
        
        bool bool_func() {
            return true;
        }
        
        string string_func() {
            return "hello";
        }
        
        void void_func() {
            return;
        }
        
        int main() {
            return int_func();
        }
    )";
    ASSERT_TRUE(analyze_code(code1));
    
    // Test wrong return type
    std::string code2 = R"(
        int func() {
            return "hello";  // Should fail
        }
    )";
    ASSERT_FALSE(analyze_code(code2));
    
    // Test missing return in non-void function
    std::string code3 = R"(
        int func() {
            int x = 5;
            // Missing return
        }
    )";
    ASSERT_FALSE(analyze_code(code3));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_semantic_complex_expressions() {
    std::cout << "Running test_semantic_complex_expressions .........";
    
    std::string code = R"(
        int main() {
            int a = 5;
            int b = 10;
            float c = 3.14;
            
            // Complex arithmetic expression
            float result1 = (a + b) * c / 2.0;
            
            // Complex boolean expression
            bool result2 = (a > 0) && (b < 20) || (c > 3.0);
            
            // Complex comparison
            bool result3 = (a + b) == (int)(c * 5);
            
            // Function call in expression
            int result4 = add(a, b) * 2;
            
            return (int)result1;
        }
        
        int add(int x, int y) {
            return x + y;
        }
    )";
    
    ASSERT_TRUE(analyze_code(code));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_semantic_list_operations_type_safety() {
    std::cout << "Running test_semantic_list_operations_type_safety .........";
    
    std::string code = R"(
        int main() {
            list<int> int_list = [1, 2, 3];
            list<float> float_list = [1.5, 2.5, 3.5];
            
            // Correct operations
            int first_int = int_list[0];
            float first_float = float_list[0];
            
            int_list.push(4);
            float_list.push(4.5);
            
            int size_int = int_list.size();
            int size_float = float_list.size();
            
            int popped_int = int_list.pop();
            float popped_float = float_list.pop();
            
            return first_int;
        }
    )";
    
    ASSERT_TRUE(analyze_code(code));
    
    std::cout << " OK" << std::endl;
    return 0;
}
