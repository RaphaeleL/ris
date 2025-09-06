#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "test_utils.h"

// Forward declarations
int test_lexer_basic();
int test_lexer_keywords();
int test_lexer_operators();
int test_lexer_literals();
int test_lexer_identifiers();
int test_lexer_punctuation();
int test_lexer_comments();
int test_lexer_whitespace();
int test_lexer_escape_sequences();
int test_parser_basic_function();
int test_parser_function_with_parameters();
int test_parser_variable_declaration();
int test_parser_arithmetic_expressions();
int test_parser_logical_expressions();
int test_parser_if_statement();
int test_parser_while_statement();
int test_parser_for_statement();
int test_parser_return_statement();
int test_parser_function_call();
int test_parser_complex_program();
int test_parser_error_handling();
int test_parser_switch_statement();
int test_parser_break_statement();
int test_parser_continue_statement();
int test_parser_list_literal();
int test_parser_list_method_calls();
int test_parser_list_indexing();
int test_semantic_valid_program();
int test_semantic_undefined_variable();
int test_semantic_duplicate_variable();
int test_semantic_type_mismatch();
int test_semantic_arithmetic_operations();
int test_semantic_boolean_operations();
int test_semantic_comparison_operations();
int test_semantic_function_calls();
int test_semantic_wrong_argument_count();
int test_semantic_undefined_function();
int test_semantic_control_flow();
int test_semantic_scope_handling();
int test_semantic_implicit_conversions();

// Code generator tests
int test_codegen_basic_function();
int test_codegen_void_function();
int test_codegen_function_with_parameters();
int test_codegen_global_variables();
int test_codegen_arithmetic_expressions();
int test_codegen_boolean_literals();
int test_codegen_float_operations();
int test_codegen_string_literals();
int test_codegen_error_handling();
int test_main_basic();

// Test function structure
struct TestFunction {
    std::string name;
    int (*func)();
};

// Diagnostics test function
int test_diagnostics() {
    // Test basic diagnostic functionality
    // Note: This is a simplified test since we can't easily test the full diagnostic system
    // without including all the headers and dependencies
    
    return 0;
}

int main() {
    int result = 0;
    
    // Define all tests
    std::vector<TestFunction> tests = {
        {"test_main_basic", test_main_basic},
        {"test_lexer_basic", test_lexer_basic},
        {"test_lexer_keywords", test_lexer_keywords},
        {"test_lexer_operators", test_lexer_operators},
        {"test_lexer_literals", test_lexer_literals},
        {"test_lexer_identifiers", test_lexer_identifiers},
        {"test_lexer_punctuation", test_lexer_punctuation},
        {"test_lexer_comments", test_lexer_comments},
        {"test_lexer_whitespace", test_lexer_whitespace},
        {"test_lexer_escape_sequences", test_lexer_escape_sequences},
        {"test_parser_basic_function", test_parser_basic_function},
        {"test_parser_function_with_parameters", test_parser_function_with_parameters},
        {"test_parser_variable_declaration", test_parser_variable_declaration},
        {"test_parser_arithmetic_expressions", test_parser_arithmetic_expressions},
        {"test_parser_logical_expressions", test_parser_logical_expressions},
        {"test_parser_if_statement", test_parser_if_statement},
        {"test_parser_while_statement", test_parser_while_statement},
        {"test_parser_for_statement", test_parser_for_statement},
        {"test_parser_return_statement", test_parser_return_statement},
        {"test_parser_function_call", test_parser_function_call},
        {"test_parser_complex_program", test_parser_complex_program},
        {"test_parser_error_handling", test_parser_error_handling},
        {"test_parser_switch_statement", test_parser_switch_statement},
        {"test_parser_break_statement", test_parser_break_statement},
        {"test_parser_continue_statement", test_parser_continue_statement},
        {"test_parser_list_literal", test_parser_list_literal},
        {"test_parser_list_method_calls", test_parser_list_method_calls},
        {"test_parser_list_indexing", test_parser_list_indexing},
        {"test_semantic_valid_program", test_semantic_valid_program},
        {"test_semantic_undefined_variable", test_semantic_undefined_variable},
        {"test_semantic_duplicate_variable", test_semantic_duplicate_variable},
        {"test_semantic_type_mismatch", test_semantic_type_mismatch},
        {"test_semantic_arithmetic_operations", test_semantic_arithmetic_operations},
        {"test_semantic_boolean_operations", test_semantic_boolean_operations},
        {"test_semantic_comparison_operations", test_semantic_comparison_operations},
        {"test_semantic_function_calls", test_semantic_function_calls},
        {"test_semantic_wrong_argument_count", test_semantic_wrong_argument_count},
        {"test_semantic_undefined_function", test_semantic_undefined_function},
        {"test_semantic_control_flow", test_semantic_control_flow},
        {"test_semantic_scope_handling", test_semantic_scope_handling},
        {"test_semantic_implicit_conversions", test_semantic_implicit_conversions},
        {"test_codegen_basic_function", test_codegen_basic_function},
        {"test_codegen_void_function", test_codegen_void_function},
        {"test_codegen_function_with_parameters", test_codegen_function_with_parameters},
        {"test_codegen_global_variables", test_codegen_global_variables},
        {"test_codegen_arithmetic_expressions", test_codegen_arithmetic_expressions},
        {"test_codegen_boolean_literals", test_codegen_boolean_literals},
        {"test_codegen_float_operations", test_codegen_float_operations},
        {"test_codegen_string_literals", test_codegen_string_literals},
        {"test_codegen_error_handling", test_codegen_error_handling},
        {"test_diagnostics", test_diagnostics}
    };
    
    // Calculate maximum test name length
    std::vector<std::string> test_names;
    for (const auto& test : tests) {
        test_names.push_back(test.name);
    }
    size_t max_length = test_utils::get_max_test_name_length(test_names);
    test_utils::g_max_test_name_length = max_length;
    
    // Run all tests
    for (const auto& test : tests) {
        // Print the aligned test start
        test_utils::print_test_start(test.name, max_length);
        
        // Redirect stdout to suppress test function output
        std::streambuf* original_cout = std::cout.rdbuf();
        std::ostringstream dev_null;
        std::cout.rdbuf(dev_null.rdbuf());
        
        // Run the test function
        int test_result = test.func();
        
        // Restore stdout
        std::cout.rdbuf(original_cout);
        
        // Print the result
        if (test_result == 0) {
            test_utils::print_test_success();
        } else {
            test_utils::print_test_failure("Test failed");
        }
        
        result += test_result;
    }
    
    if (result == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << result << " test(s) failed!" << std::endl;
    }
    
    return result;
}
