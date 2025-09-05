#include <iostream>

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
int test_parser_array_declaration();
int test_parser_arithmetic_expressions();
int test_parser_logical_expressions();
int test_parser_if_statement();
int test_parser_while_statement();
int test_parser_for_statement();
int test_parser_return_statement();
int test_parser_function_call();
int test_parser_array_access();
int test_parser_complex_program();
int test_parser_error_handling();
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
int test_semantic_array_operations();
int test_semantic_scope_handling();
int test_semantic_implicit_conversions();
int test_main_basic();

int main() {
    int result = 0;
    
    result += test_main_basic();
    result += test_lexer_basic();
    result += test_lexer_keywords();
    result += test_lexer_operators();
    result += test_lexer_literals();
    result += test_lexer_identifiers();
    result += test_lexer_punctuation();
    result += test_lexer_comments();
    result += test_lexer_whitespace();
    result += test_lexer_escape_sequences();
    result += test_parser_basic_function();
    result += test_parser_function_with_parameters();
    result += test_parser_variable_declaration();
    result += test_parser_array_declaration();
    result += test_parser_arithmetic_expressions();
    result += test_parser_logical_expressions();
    result += test_parser_if_statement();
    result += test_parser_while_statement();
    result += test_parser_for_statement();
    result += test_parser_return_statement();
    result += test_parser_function_call();
    result += test_parser_array_access();
    result += test_parser_complex_program();
    result += test_parser_error_handling();
    result += test_semantic_valid_program();
    result += test_semantic_undefined_variable();
    result += test_semantic_duplicate_variable();
    result += test_semantic_type_mismatch();
    result += test_semantic_arithmetic_operations();
    result += test_semantic_boolean_operations();
    result += test_semantic_comparison_operations();
    result += test_semantic_function_calls();
    result += test_semantic_wrong_argument_count();
    result += test_semantic_undefined_function();
    result += test_semantic_control_flow();
    result += test_semantic_array_operations();
    result += test_semantic_scope_handling();
    result += test_semantic_implicit_conversions();
    
    if (result == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << result << " test(s) failed!" << std::endl;
    }
    
    return result;
}
