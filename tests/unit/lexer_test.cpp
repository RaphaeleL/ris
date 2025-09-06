#include "lexer.h"
#include <iostream>
#include <cassert>
#include <vector>

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

int test_lexer_basic() {
    std::cout << "Running test_lexer_basic .........";
    
    ris::Lexer lexer("int main() { return 42; }");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(10, tokens.size()); // 9 tokens + EOF
    ASSERT_EQ(ris::TokenType::INT, tokens[0].type);
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[1].type);
    ASSERT_EQ("main", tokens[1].value);
    ASSERT_EQ(ris::TokenType::LEFT_PAREN, tokens[2].type);
    ASSERT_EQ(ris::TokenType::RIGHT_PAREN, tokens[3].type);
    ASSERT_EQ(ris::TokenType::LEFT_BRACE, tokens[4].type);
    ASSERT_EQ(ris::TokenType::RETURN, tokens[5].type);
    ASSERT_EQ(ris::TokenType::INTEGER_LITERAL, tokens[6].type);
    ASSERT_EQ("42", tokens[6].value);
    ASSERT_EQ(ris::TokenType::SEMICOLON, tokens[7].type);
    ASSERT_EQ(ris::TokenType::RIGHT_BRACE, tokens[8].type);
    
    
    return 0;
}

int test_lexer_keywords() {
    std::cout << "Running test_lexer_keywords .........";
    
    ris::Lexer lexer("int float bool char string if else while for break continue return true false");
    auto tokens = lexer.tokenize();
    
    std::vector<ris::TokenType> expected_types = {
        ris::TokenType::INT, ris::TokenType::FLOAT, ris::TokenType::BOOL, ris::TokenType::CHAR,
        ris::TokenType::STRING, ris::TokenType::IF, ris::TokenType::ELSE, ris::TokenType::WHILE,
        ris::TokenType::FOR, ris::TokenType::BREAK, ris::TokenType::CONTINUE, ris::TokenType::RETURN,
        ris::TokenType::TRUE, ris::TokenType::FALSE
    };
    
    ASSERT_EQ(expected_types.size() + 1, tokens.size()); // +1 for EOF
    
    for (size_t i = 0; i < expected_types.size(); i++) {
        ASSERT_EQ(expected_types[i], tokens[i].type);
    }
    
    
    return 0;
}

int test_lexer_operators() {
    std::cout << "Running test_lexer_operators .........";
    
    ris::Lexer lexer("+ - * / % == != < > <= >= && || ! =");
    auto tokens = lexer.tokenize();
    
    std::vector<ris::TokenType> expected_types = {
        ris::TokenType::PLUS, ris::TokenType::MINUS, ris::TokenType::MULTIPLY, ris::TokenType::DIVIDE,
        ris::TokenType::MODULO, ris::TokenType::EQUAL, ris::TokenType::NOT_EQUAL, ris::TokenType::LESS,
        ris::TokenType::GREATER, ris::TokenType::LESS_EQUAL, ris::TokenType::GREATER_EQUAL,
        ris::TokenType::AND, ris::TokenType::OR, ris::TokenType::NOT, ris::TokenType::ASSIGN
    };
    
    ASSERT_EQ(expected_types.size() + 1, tokens.size()); // +1 for EOF
    
    for (size_t i = 0; i < expected_types.size(); i++) {
        ASSERT_EQ(expected_types[i], tokens[i].type);
    }
    
    
    return 0;
}

int test_lexer_literals() {
    std::cout << "Running test_lexer_literals .........";
    
    ris::Lexer lexer("123 3.14 'a' \"hello\"");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(5, tokens.size()); // 4 literals + EOF
    
    ASSERT_EQ(ris::TokenType::INTEGER_LITERAL, tokens[0].type);
    ASSERT_EQ("123", tokens[0].value);
    
    ASSERT_EQ(ris::TokenType::FLOAT_LITERAL, tokens[1].type);
    ASSERT_EQ("3.14", tokens[1].value);
    
    ASSERT_EQ(ris::TokenType::CHAR_LITERAL, tokens[2].type);
    ASSERT_EQ("a", tokens[2].value);
    
    ASSERT_EQ(ris::TokenType::STRING_LITERAL, tokens[3].type);
    ASSERT_EQ("hello", tokens[3].value);
    
    
    return 0;
}

int test_lexer_identifiers() {
    std::cout << "Running test_lexer_identifiers .........";
    
    ris::Lexer lexer("variable_name _underscore var123");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(4, tokens.size()); // 3 identifiers + EOF
    
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[0].type);
    ASSERT_EQ("variable_name", tokens[0].value);
    
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[1].type);
    ASSERT_EQ("_underscore", tokens[1].value);
    
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[2].type);
    ASSERT_EQ("var123", tokens[2].value);
    
    
    return 0;
}

int test_lexer_punctuation() {
    std::cout << "Running test_lexer_punctuation .........";
    
    ris::Lexer lexer("; , . ( ) { } [ ]");
    auto tokens = lexer.tokenize();
    
    std::vector<ris::TokenType> expected_types = {
        ris::TokenType::SEMICOLON, ris::TokenType::COMMA, ris::TokenType::DOT,
        ris::TokenType::LEFT_PAREN, ris::TokenType::RIGHT_PAREN,
        ris::TokenType::LEFT_BRACE, ris::TokenType::RIGHT_BRACE,
        ris::TokenType::LEFT_BRACKET, ris::TokenType::RIGHT_BRACKET
    };
    
    ASSERT_EQ(expected_types.size() + 1, tokens.size()); // +1 for EOF
    
    for (size_t i = 0; i < expected_types.size(); i++) {
        ASSERT_EQ(expected_types[i], tokens[i].type);
    }
    
    
    return 0;
}

int test_lexer_comments() {
    std::cout << "Running test_lexer_comments .........";
    
    ris::Lexer lexer("int x; // comment\nint y; /* multi\nline */ int z;");
    auto tokens = lexer.tokenize();
    
    // Should have: int, x, ;, int, y, ;, int, z, ;, EOF
    ASSERT_EQ(10, tokens.size());
    
    ASSERT_EQ(ris::TokenType::INT, tokens[0].type);
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[1].type);
    ASSERT_EQ("x", tokens[1].value);
    ASSERT_EQ(ris::TokenType::SEMICOLON, tokens[2].type);
    ASSERT_EQ(ris::TokenType::INT, tokens[3].type);
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[4].type);
    ASSERT_EQ("y", tokens[4].value);
    ASSERT_EQ(ris::TokenType::SEMICOLON, tokens[5].type);
    ASSERT_EQ(ris::TokenType::INT, tokens[6].type);
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[7].type);
    ASSERT_EQ("z", tokens[7].value);
    ASSERT_EQ(ris::TokenType::SEMICOLON, tokens[8].type);
    
    
    return 0;
}

int test_lexer_whitespace() {
    std::cout << "Running test_lexer_whitespace .........";
    
    ris::Lexer lexer("  int   x  =  42  ;  ");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(6, tokens.size()); // int, x, =, 42, ;, EOF
    
    ASSERT_EQ(ris::TokenType::INT, tokens[0].type);
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[1].type);
    ASSERT_EQ("x", tokens[1].value);
    ASSERT_EQ(ris::TokenType::ASSIGN, tokens[2].type);
    ASSERT_EQ(ris::TokenType::INTEGER_LITERAL, tokens[3].type);
    ASSERT_EQ("42", tokens[3].value);
    ASSERT_EQ(ris::TokenType::SEMICOLON, tokens[4].type);
    
    
    return 0;
}

int test_lexer_escape_sequences() {
    std::cout << "Running test_lexer_escape_sequences .........";
    
    ris::Lexer lexer("'\\n' '\\t' '\\r' '\\\\' '\\'' '\\\"' \"hello\\nworld\"");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(8, tokens.size()); // 6 char literals + 1 string literal + EOF
    
    ASSERT_EQ(ris::TokenType::CHAR_LITERAL, tokens[0].type);
    ASSERT_EQ("\n", tokens[0].value);
    
    ASSERT_EQ(ris::TokenType::CHAR_LITERAL, tokens[1].type);
    ASSERT_EQ("\t", tokens[1].value);
    
    ASSERT_EQ(ris::TokenType::CHAR_LITERAL, tokens[2].type);
    ASSERT_EQ("\r", tokens[2].value);
    
    ASSERT_EQ(ris::TokenType::CHAR_LITERAL, tokens[3].type);
    ASSERT_EQ("\\", tokens[3].value);
    
    ASSERT_EQ(ris::TokenType::CHAR_LITERAL, tokens[4].type);
    ASSERT_EQ("'", tokens[4].value);
    
    ASSERT_EQ(ris::TokenType::CHAR_LITERAL, tokens[5].type);
    ASSERT_EQ("\"", tokens[5].value);
    
    ASSERT_EQ(ris::TokenType::STRING_LITERAL, tokens[6].type);
    ASSERT_EQ("hello\nworld", tokens[6].value);
    
    
    return 0;
}

// Additional comprehensive lexer tests
int test_lexer_all_keywords() {
    std::cout << "Running test_lexer_all_keywords .........";
    
    std::string all_keywords = "int float bool char string void list if else while for switch case default break continue return true false";
    ris::Lexer lexer(all_keywords);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(18, tokens.size()); // 17 keywords + EOF
    
    std::vector<ris::TokenType> expected_types = {
        ris::TokenType::INT, ris::TokenType::FLOAT, ris::TokenType::BOOL, ris::TokenType::CHAR,
        ris::TokenType::STRING, ris::TokenType::VOID, ris::TokenType::LIST, ris::TokenType::IF,
        ris::TokenType::ELSE, ris::TokenType::WHILE, ris::TokenType::FOR, ris::TokenType::SWITCH,
        ris::TokenType::CASE, ris::TokenType::DEFAULT, ris::TokenType::BREAK, ris::TokenType::CONTINUE,
        ris::TokenType::RETURN, ris::TokenType::TRUE, ris::TokenType::FALSE
    };
    
    for (size_t i = 0; i < expected_types.size(); ++i) {
        ASSERT_EQ(expected_types[i], tokens[i].type);
    }
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_lexer_all_operators() {
    std::cout << "Running test_lexer_all_operators .........";
    
    std::string all_operators = "+ - * / % == != < > <= >= && || ! = ++";
    ris::Lexer lexer(all_operators);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(15, tokens.size()); // 14 operators + EOF
    
    std::vector<ris::TokenType> expected_types = {
        ris::TokenType::PLUS, ris::TokenType::MINUS, ris::TokenType::MULTIPLY, ris::TokenType::DIVIDE,
        ris::TokenType::MODULO, ris::TokenType::EQUAL, ris::TokenType::NOT_EQUAL, ris::TokenType::LESS,
        ris::TokenType::GREATER, ris::TokenType::LESS_EQUAL, ris::TokenType::GREATER_EQUAL,
        ris::TokenType::AND, ris::TokenType::OR, ris::TokenType::NOT, ris::TokenType::ASSIGN,
        ris::TokenType::INCREMENT
    };
    
    for (size_t i = 0; i < expected_types.size(); ++i) {
        ASSERT_EQ(expected_types[i], tokens[i].type);
    }
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_lexer_all_punctuation() {
    std::cout << "Running test_lexer_all_punctuation .........";
    
    std::string all_punctuation = "; , . : ( ) { } [ ]";
    ris::Lexer lexer(all_punctuation);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(11, tokens.size()); // 10 punctuation + EOF
    
    std::vector<ris::TokenType> expected_types = {
        ris::TokenType::SEMICOLON, ris::TokenType::COMMA, ris::TokenType::DOT, ris::TokenType::COLON,
        ris::TokenType::LEFT_PAREN, ris::TokenType::RIGHT_PAREN, ris::TokenType::LEFT_BRACE,
        ris::TokenType::RIGHT_BRACE, ris::TokenType::LEFT_BRACKET, ris::TokenType::RIGHT_BRACKET
    };
    
    for (size_t i = 0; i < expected_types.size(); ++i) {
        ASSERT_EQ(expected_types[i], tokens[i].type);
    }
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_lexer_numeric_literals() {
    std::cout << "Running test_lexer_numeric_literals .........";
    
    ris::Lexer lexer("42 3.14 0x1A 0b1010 1e5 2.5e-3");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(7, tokens.size()); // 6 literals + EOF
    
    ASSERT_EQ(ris::TokenType::INTEGER_LITERAL, tokens[0].type);
    ASSERT_EQ("42", tokens[0].value);
    
    ASSERT_EQ(ris::TokenType::FLOAT_LITERAL, tokens[1].type);
    ASSERT_EQ("3.14", tokens[1].value);
    
    ASSERT_EQ(ris::TokenType::INTEGER_LITERAL, tokens[2].type);
    ASSERT_EQ("0x1A", tokens[2].value);
    
    ASSERT_EQ(ris::TokenType::INTEGER_LITERAL, tokens[3].type);
    ASSERT_EQ("0b1010", tokens[3].value);
    
    ASSERT_EQ(ris::TokenType::FLOAT_LITERAL, tokens[4].type);
    ASSERT_EQ("1e5", tokens[4].value);
    
    ASSERT_EQ(ris::TokenType::FLOAT_LITERAL, tokens[5].type);
    ASSERT_EQ("2.5e-3", tokens[5].value);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_lexer_string_variations() {
    std::cout << "Running test_lexer_string_variations .........";
    
    ris::Lexer lexer(R"("hello" 'a' "escaped\"quote" '\\' "multiline\nstring")");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(6, tokens.size()); // 5 literals + EOF
    
    ASSERT_EQ(ris::TokenType::STRING_LITERAL, tokens[0].type);
    ASSERT_EQ("hello", tokens[0].value);
    
    ASSERT_EQ(ris::TokenType::CHAR_LITERAL, tokens[1].type);
    ASSERT_EQ("a", tokens[1].value);
    
    ASSERT_EQ(ris::TokenType::STRING_LITERAL, tokens[2].type);
    ASSERT_EQ("escaped\"quote", tokens[2].value);
    
    ASSERT_EQ(ris::TokenType::CHAR_LITERAL, tokens[3].type);
    ASSERT_EQ("\\", tokens[3].value);
    
    ASSERT_EQ(ris::TokenType::STRING_LITERAL, tokens[4].type);
    ASSERT_EQ("multiline\nstring", tokens[4].value);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_lexer_identifiers_edge_cases() {
    std::cout << "Running test_lexer_identifiers_edge_cases .........";
    
    ris::Lexer lexer("_underscore _123 camelCase snake_case UPPERCASE mixed123Case");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(7, tokens.size()); // 6 identifiers + EOF
    
    for (int i = 0; i < 6; ++i) {
        ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[i].type);
    }
    
    ASSERT_EQ("_underscore", tokens[0].value);
    ASSERT_EQ("_123", tokens[1].value);
    ASSERT_EQ("camelCase", tokens[2].value);
    ASSERT_EQ("snake_case", tokens[3].value);
    ASSERT_EQ("UPPERCASE", tokens[4].value);
    ASSERT_EQ("mixed123Case", tokens[5].value);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_lexer_comments_comprehensive() {
    std::cout << "Running test_lexer_comments_comprehensive .........";
    
    std::string code = R"(
        // Single line comment
        int x = 5; // Inline comment
        /* Multi-line
           comment */
        int y = 10;
        /* Single line block comment */
    )";
    
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    // Should only have: int, x, =, 5, ;, int, y, =, 10, ;, EOF
    ASSERT_EQ(11, tokens.size());
    
    ASSERT_EQ(ris::TokenType::INT, tokens[0].type);
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[1].type);
    ASSERT_EQ("x", tokens[1].value);
    ASSERT_EQ(ris::TokenType::ASSIGN, tokens[2].type);
    ASSERT_EQ(ris::TokenType::INTEGER_LITERAL, tokens[3].type);
    ASSERT_EQ("5", tokens[3].value);
    ASSERT_EQ(ris::TokenType::SEMICOLON, tokens[4].type);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_lexer_preprocessor_directives() {
    std::cout << "Running test_lexer_preprocessor_directives .........";
    
    ris::Lexer lexer("#include <std>\n#include \"local.h\"");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(5, tokens.size()); // HASH, INCLUDE, SYSTEM_INCLUDE, HASH, INCLUDE, EOF
    
    ASSERT_EQ(ris::TokenType::HASH, tokens[0].type);
    ASSERT_EQ(ris::TokenType::INCLUDE, tokens[1].type);
    ASSERT_EQ(ris::TokenType::SYSTEM_INCLUDE, tokens[2].type);
    ASSERT_EQ("std", tokens[2].value);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_lexer_error_conditions() {
    std::cout << "Running test_lexer_error_conditions .........";
    
    // Test unterminated string
    ris::Lexer lexer1("\"unterminated string");
    auto tokens1 = lexer1.tokenize();
    ASSERT_TRUE(lexer1.has_error());
    
    // Test unterminated char
    ris::Lexer lexer2("'a");
    auto tokens2 = lexer2.tokenize();
    ASSERT_TRUE(lexer2.has_error());
    
    // Test unterminated block comment
    ris::Lexer lexer3("/* unterminated comment");
    auto tokens3 = lexer3.tokenize();
    ASSERT_TRUE(lexer3.has_error());
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_lexer_position_tracking() {
    std::cout << "Running test_lexer_position_tracking .........";
    
    std::string code = "int x = 5;\nfloat y = 3.14;";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    // Check line numbers
    ASSERT_EQ(1, tokens[0].position.line); // int
    ASSERT_EQ(1, tokens[1].position.line); // x
    ASSERT_EQ(1, tokens[2].position.line); // =
    ASSERT_EQ(1, tokens[3].position.line); // 5
    ASSERT_EQ(1, tokens[4].position.line); // ;
    ASSERT_EQ(2, tokens[5].position.line); // float
    ASSERT_EQ(2, tokens[6].position.line); // y
    ASSERT_EQ(2, tokens[7].position.line); // =
    ASSERT_EQ(2, tokens[8].position.line); // 3.14
    ASSERT_EQ(2, tokens[9].position.line); // ;
    
    std::cout << " OK" << std::endl;
    return 0;
}

// Test functions are defined above, main() is in test_runner.cpp
