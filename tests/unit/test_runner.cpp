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
    
    if (result == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << result << " test(s) failed!" << std::endl;
    }
    
    return result;
}
