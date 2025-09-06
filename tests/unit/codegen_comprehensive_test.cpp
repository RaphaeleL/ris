#include <iostream>
#include <string>
#include <fstream>
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "codegen.h"
#include "test_utils.h"

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << " FAIL  " #condition " is false at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            std::cerr << " FAIL  " #condition " is true at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while(0)

// Helper function to compile and check if code generation succeeds
static bool compile_code_comprehensive(const std::string& code, std::string& output_file) {
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    if (parser.has_error()) {
        return false;
    }
    
    ris::SemanticAnalyzer analyzer;
    if (!analyzer.analyze(*program)) {
        return false;
    }
    
    ris::CodeGenerator codegen;
    output_file = "test_output_comprehensive.ll";
    return codegen.generate(std::move(program), output_file);
}

// Helper function to check if generated file exists and contains expected content
static bool check_file_contains_comprehensive(const std::string& filename, const std::string& expected) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    return content.find(expected) != std::string::npos;
}

int test_codegen_control_flow_comprehensive() {
    std::cout << "Running test_codegen_control_flow_comprehensive .........";
    
    std::string code = R"(
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
                if (i % 2 == 0) {
                    continue;
                }
                if (i > 5) {
                    break;
                }
            }
            
            return x;
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define i64 @main()"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "br"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "phi"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_switch_statement() {
    std::cout << "Running test_codegen_switch_statement .........";
    
    std::string code = R"(
        int main() {
            int x = 2;
            
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
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define i64 @main()"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "switch"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_function_calls() {
    std::cout << "Running test_codegen_function_calls .........";
    
    std::string code = R"(
        int add(int a, int b) {
            return a + b;
        }
        
        int multiply(int a, int b) {
            return a * b;
        }
        
        int main() {
            int x = add(5, 3);
            int y = multiply(x, 2);
            return add(x, y);
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define i64 @add(i64, i64)"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define i64 @multiply(i64, i64)"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "call i64 @add"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "call i64 @multiply"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_global_variables_comprehensive() {
    std::cout << "Running test_codegen_global_variables .........";
    
    std::string code = R"(
        int global_int = 42;
        float global_float = 3.14;
        bool global_bool = true;
        
        int main() {
            int local = global_int;
            float local_float = global_float;
            bool local_bool = global_bool;
            return local;
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "@global_int"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "@global_float"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "@global_bool"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_list_operations() {
    std::cout << "Running test_codegen_list_operations .........";
    
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
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define i64 @main()"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "call void @ris_list_push"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "call i64 @ris_list_size"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "call i64 @ris_list_pop"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_arithmetic_operations() {
    std::cout << "Running test_codegen_arithmetic_operations .........";
    
    std::string code = R"(
        int main() {
            int a = 10;
            int b = 3;
            
            int add_result = a + b;
            int sub_result = a - b;
            int mul_result = a * b;
            int div_result = a / b;
            int mod_result = a % b;
            
            return add_result + sub_result + mul_result + div_result + mod_result;
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "add"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "sub"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "mul"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "sdiv"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "srem"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_float_operations_comprehensive() {
    std::cout << "Running test_codegen_float_operations .........";
    
    std::string code = R"(
        float main() {
            float a = 10.5;
            float b = 3.2;
            
            float add_result = a + b;
            float sub_result = a - b;
            float mul_result = a * b;
            float div_result = a / b;
            
            return add_result + sub_result + mul_result + div_result;
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define double @main()"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "fadd"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "fsub"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "fmul"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "fdiv"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_boolean_operations() {
    std::cout << "Running test_codegen_boolean_operations .........";
    
    std::string code = R"(
        bool main() {
            bool a = true;
            bool b = false;
            
            bool and_result = a && b;
            bool or_result = a || b;
            bool not_result = !a;
            
            return and_result || or_result || not_result;
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define i8 @main()"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "and"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "or"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "xor"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_comparison_operations() {
    std::cout << "Running test_codegen_comparison_operations .........";
    
    std::string code = R"(
        bool main() {
            int a = 10;
            int b = 5;
            
            bool eq = a == b;
            bool ne = a != b;
            bool lt = a < b;
            bool le = a <= b;
            bool gt = a > b;
            bool ge = a >= b;
            
            return eq || ne || lt || le || gt || ge;
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define i8 @main()"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "icmp eq"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "icmp ne"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "icmp slt"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "icmp sle"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "icmp sgt"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "icmp sge"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_string_operations() {
    std::cout << "Running test_codegen_string_operations .........";
    
    std::string code = R"(
        string main() {
            string hello = "Hello";
            string world = "World";
            string combined = hello + " " + world;
            return combined;
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define %struct.string* @main()"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "call %struct.string* @ris_string_concat"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_increment_operators() {
    std::cout << "Running test_codegen_increment_operators .........";
    
    std::string code = R"(
        int main() {
            int x = 5;
            ++x;
            x++;
            return x;
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define i64 @main()"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "add"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_complex_expressions() {
    std::cout << "Running test_codegen_complex_expressions .........";
    
    std::string code = R"(
        int main() {
            int a = 5;
            int b = 10;
            int c = 3;
            
            int result = (a + b) * c - (a / c) + (b % c);
            bool condition = (a > 0) && (b < 20) || (c == 3);
            
            return condition ? result : 0;
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define i64 @main()"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "select"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_recursive_functions() {
    std::cout << "Running test_codegen_recursive_functions .........";
    
    std::string code = R"(
        int factorial(int n) {
            if (n <= 1) {
                return 1;
            }
            return n * factorial(n - 1);
        }
        
        int main() {
            return factorial(5);
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "define i64 @factorial(i64)"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "call i64 @factorial"));
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_codegen_memory_management() {
    std::cout << "Running test_codegen_memory_management .........";
    
    std::string code = R"(
        int main() {
            list<int> numbers = [1, 2, 3, 4, 5];
            string text = "Hello World";
            
            numbers.push(6);
            int size = numbers.size();
            int popped = numbers.pop();
            
            return size + popped;
        }
    )";
    
    std::string output_file;
    ASSERT_TRUE(compile_code_comprehensive(code, output_file));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "call i8* @ris_malloc"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "call void @ris_list_push"));
    ASSERT_TRUE(check_file_contains_comprehensive(output_file, "call i64 @ris_list_pop"));
    
    std::cout << " OK" << std::endl;
    return 0;
}
