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
    
    if (result == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << result << " test(s) failed!" << std::endl;
    }
    
    return result;
}
