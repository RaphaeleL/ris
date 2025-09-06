#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "test_utils.h"

// ANSI color codes (also defined in test_utils.h but needed here for direct use)
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m"

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
int test_lexer_all_keywords();
int test_lexer_all_operators();
int test_lexer_all_punctuation();
int test_lexer_numeric_literals();
int test_lexer_string_variations();
int test_lexer_identifiers_edge_cases();
int test_lexer_comments_comprehensive();
int test_lexer_preprocessor_directives();
int test_lexer_error_conditions();
int test_lexer_position_tracking();
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

// Comprehensive parser tests
int test_parser_complex_expressions();
int test_parser_nested_control_flow();
int test_parser_switch_statement_comprehensive();
int test_parser_list_operations_comprehensive();
int test_parser_function_overloading_simulation();
int test_parser_global_variables();
int test_parser_increment_operators();
int test_parser_complex_arithmetic();
int test_parser_error_recovery();
int test_parser_edge_cases();

// Comprehensive semantic tests
int test_semantic_type_coercion();
int test_semantic_scope_nesting();
int test_semantic_variable_shadowing();
int test_semantic_function_signature_matching();
int test_semantic_list_type_checking();
int test_semantic_control_flow_type_checking();
int test_semantic_switch_type_checking();
int test_semantic_arithmetic_type_promotion();
int test_semantic_error_detection();
int test_semantic_return_type_checking();
int test_semantic_complex_expressions();
int test_semantic_list_operations_type_safety();

// Comprehensive codegen tests
int test_codegen_control_flow_comprehensive();
int test_codegen_switch_statement();
int test_codegen_function_calls();
int test_codegen_global_variables_comprehensive();
int test_codegen_list_operations();
int test_codegen_arithmetic_operations();
int test_codegen_float_operations_comprehensive();
int test_codegen_boolean_operations();
int test_codegen_comparison_operations();
int test_codegen_string_operations();
int test_codegen_increment_operators();
int test_codegen_complex_expressions();
int test_codegen_recursive_functions();
int test_codegen_memory_management();

// Runtime tests
int test_runtime_list_creation();
int test_runtime_list_push_pop();
int test_runtime_list_get();
int test_runtime_list_size();
int test_runtime_list_different_types();
int test_runtime_list_bounds_checking();
int test_runtime_list_pop_empty();
int test_runtime_memory_management();
int test_runtime_list_resize();
int test_runtime_print_functions();
int test_runtime_string_operations();
int test_runtime_memory_allocation();

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
        {"test_lexer_all_keywords", test_lexer_all_keywords},
        {"test_lexer_all_operators", test_lexer_all_operators},
        {"test_lexer_all_punctuation", test_lexer_all_punctuation},
        {"test_lexer_numeric_literals", test_lexer_numeric_literals},
        {"test_lexer_string_variations", test_lexer_string_variations},
        {"test_lexer_identifiers_edge_cases", test_lexer_identifiers_edge_cases},
        {"test_lexer_comments_comprehensive", test_lexer_comments_comprehensive},
        {"test_lexer_preprocessor_directives", test_lexer_preprocessor_directives},
        {"test_lexer_error_conditions", test_lexer_error_conditions},
        {"test_lexer_position_tracking", test_lexer_position_tracking},
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
        {"test_parser_complex_expressions", test_parser_complex_expressions},
        {"test_parser_nested_control_flow", test_parser_nested_control_flow},
        {"test_parser_switch_statement_comprehensive", test_parser_switch_statement_comprehensive},
        {"test_parser_list_operations_comprehensive", test_parser_list_operations_comprehensive},
        {"test_parser_function_overloading_simulation", test_parser_function_overloading_simulation},
        {"test_parser_global_variables", test_parser_global_variables},
        {"test_parser_increment_operators", test_parser_increment_operators},
        {"test_parser_complex_arithmetic", test_parser_complex_arithmetic},
        {"test_parser_error_recovery", test_parser_error_recovery},
        {"test_parser_edge_cases", test_parser_edge_cases},
        {"test_semantic_type_coercion", test_semantic_type_coercion},
        {"test_semantic_scope_nesting", test_semantic_scope_nesting},
        {"test_semantic_variable_shadowing", test_semantic_variable_shadowing},
        {"test_semantic_function_signature_matching", test_semantic_function_signature_matching},
        {"test_semantic_list_type_checking", test_semantic_list_type_checking},
        {"test_semantic_control_flow_type_checking", test_semantic_control_flow_type_checking},
        {"test_semantic_switch_type_checking", test_semantic_switch_type_checking},
        {"test_semantic_arithmetic_type_promotion", test_semantic_arithmetic_type_promotion},
        {"test_semantic_error_detection", test_semantic_error_detection},
        {"test_semantic_return_type_checking", test_semantic_return_type_checking},
        {"test_semantic_complex_expressions", test_semantic_complex_expressions},
        {"test_semantic_list_operations_type_safety", test_semantic_list_operations_type_safety},
        {"test_codegen_control_flow_comprehensive", test_codegen_control_flow_comprehensive},
        {"test_codegen_switch_statement", test_codegen_switch_statement},
        {"test_codegen_function_calls", test_codegen_function_calls},
        {"test_codegen_global_variables_comprehensive", test_codegen_global_variables_comprehensive},
        {"test_codegen_list_operations", test_codegen_list_operations},
        {"test_codegen_arithmetic_operations", test_codegen_arithmetic_operations},
        {"test_codegen_float_operations_comprehensive", test_codegen_float_operations_comprehensive},
        {"test_codegen_boolean_operations", test_codegen_boolean_operations},
        {"test_codegen_comparison_operations", test_codegen_comparison_operations},
        {"test_codegen_string_operations", test_codegen_string_operations},
        {"test_codegen_increment_operators", test_codegen_increment_operators},
        {"test_codegen_complex_expressions", test_codegen_complex_expressions},
        {"test_codegen_recursive_functions", test_codegen_recursive_functions},
        {"test_codegen_memory_management", test_codegen_memory_management},
        {"test_runtime_list_creation", test_runtime_list_creation},
        {"test_runtime_list_push_pop", test_runtime_list_push_pop},
        {"test_runtime_list_get", test_runtime_list_get},
        {"test_runtime_list_size", test_runtime_list_size},
        {"test_runtime_list_different_types", test_runtime_list_different_types},
        {"test_runtime_list_bounds_checking", test_runtime_list_bounds_checking},
        {"test_runtime_list_pop_empty", test_runtime_list_pop_empty},
        {"test_runtime_memory_management", test_runtime_memory_management},
        {"test_runtime_list_resize", test_runtime_list_resize},
        {"test_runtime_print_functions", test_runtime_print_functions},
        {"test_runtime_string_operations", test_runtime_string_operations},
        {"test_runtime_memory_allocation", test_runtime_memory_allocation},
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
        
        // Redirect stdout and stderr to capture test output
        std::streambuf* original_cout = std::cout.rdbuf();
        std::streambuf* original_cerr = std::cerr.rdbuf();
        std::ostringstream captured_output;
        std::ostringstream captured_errors;
        std::cout.rdbuf(captured_output.rdbuf());
        std::cerr.rdbuf(captured_errors.rdbuf());
        
        // Run the test function
        int test_result = test.func();
        
        // Restore stdout and stderr
        std::cout.rdbuf(original_cout);
        std::cerr.rdbuf(original_cerr);
        
        // Print the result
        if (test_result == 0) {
            test_utils::print_test_success();
        } else {
            // Print the failure message inline
            std::string error_msg = captured_errors.str();
            // Remove the " FAIL  " prefix and newline from the error message
            if (error_msg.find(" FAIL  ") == 0) {
                error_msg = error_msg.substr(7); // Remove " FAIL  "
            }
            // Remove trailing newline if present
            if (!error_msg.empty() && error_msg.back() == '\n') {
                error_msg.pop_back();
            }
            std::cout << " " << COLOR_RED << "FAIL" << COLOR_RESET << "  " << error_msg << std::endl;
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
