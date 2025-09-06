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
    } while (0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            std::cerr << " FAIL  " #condition " is true at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while (0)

// Helper function to compile and check if code generation succeeds
bool compile_code(const std::string& code, std::string& output_file) {
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
    output_file = "test_output.ll";
    return codegen.generate(std::move(program), output_file);
}

// Helper function to check if generated file exists and contains expected content
bool check_file_contains(const std::string& filename, const std::string& expected) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    return content.find(expected) != std::string::npos;
}

int test_codegen_basic_function() {
    std::string code = "int main() { return 42; }";
    std::string output_file;
    
    ASSERT_TRUE(compile_code(code, output_file));
    ASSERT_TRUE(check_file_contains(output_file, "define i64 @main()"));
    ASSERT_TRUE(check_file_contains(output_file, "ret i64 42"));
    
    return 0;
}

int test_codegen_void_function() {
    std::string code = "void test() { return; }";
    std::string output_file;
    
    ASSERT_TRUE(compile_code(code, output_file));
    ASSERT_TRUE(check_file_contains(output_file, "define void @test()"));
    ASSERT_TRUE(check_file_contains(output_file, "ret void"));
    return 0;
}

int test_codegen_function_with_parameters() {
    std::cout << "Running test_codegen_function_with_parameters .........";
    
    std::string code = "int add(int a, int b) { return a; }";
    std::string output_file;
    
    ASSERT_TRUE(compile_code(code, output_file));
    ASSERT_TRUE(check_file_contains(output_file, "define i64 @add(i64"));
    
    
    return 0;
}

int test_codegen_global_variables() {
    std::cout << "Running test_codegen_global_variables .........";
    
    std::string code = "int global_var = 123; int main() { return 0; }";
    std::string output_file;
    
    ASSERT_TRUE(compile_code(code, output_file));
    ASSERT_TRUE(check_file_contains(output_file, "@global_var"));
    
    
    return 0;
}

int test_codegen_arithmetic_expressions() {
    std::string code = "int main() { return 2 + 3; }";
    std::string output_file;
    
    ASSERT_TRUE(compile_code(code, output_file));
    // LLVM might constant fold 2 + 3 to 5, so check for the result or the operation
    ASSERT_TRUE(check_file_contains(output_file, "ret i64 5") || check_file_contains(output_file, "add"));
    
    return 0;
}

int test_codegen_boolean_literals() {
    std::string code = "bool main() { return true; }";
    std::string output_file;
    
    ASSERT_TRUE(compile_code(code, output_file));
    ASSERT_TRUE(check_file_contains(output_file, "define i8 @main()"));
    ASSERT_TRUE(check_file_contains(output_file, "ret i8 1"));
    
    return 0;
}

int test_codegen_float_operations() {
    std::string code = "float main() { return 3.14; }";
    std::string output_file;
    
    ASSERT_TRUE(compile_code(code, output_file));
    ASSERT_TRUE(check_file_contains(output_file, "define double @main()"));
    ASSERT_TRUE(check_file_contains(output_file, "ret double"));
    
    return 0;
}

int test_codegen_string_literals() {
    std::string code = R"(string main() { return "hello"; })";
    std::string output_file;
    
    ASSERT_TRUE(compile_code(code, output_file));
    ASSERT_TRUE(check_file_contains(output_file, "hello"));
    
    // Temporarily force a failure to test colors
    ASSERT_TRUE(false);
    
    return 0;
}

int test_codegen_error_handling() {
    std::cout << "Running test_codegen_error_handling .........";
    
    // Test with invalid code that should fail semantic analysis
    std::string code = "int main() { return undefined_var; }";
    std::string output_file;
    
    ASSERT_FALSE(compile_code(code, output_file));
    
    
    return 0;
}

// Test runner functions (will be called from test_runner.cpp)
int test_codegen_basic_function();
int test_codegen_void_function();
int test_codegen_function_with_parameters();
int test_codegen_global_variables();
int test_codegen_arithmetic_expressions();
int test_codegen_boolean_literals();
int test_codegen_float_operations();
int test_codegen_string_literals();
int test_codegen_error_handling();
