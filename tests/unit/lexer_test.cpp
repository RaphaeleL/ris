#include "lexer.h"
#include <iostream>
#include <cassert>
#include <vector>
#include "test_utils.h"

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

// Comprehensive lexer tests for all token types and edge cases

int test_lexer_all_keywords() {
    std::string code = "int float bool char string void list if else while for switch case default break continue return true false";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(19, tokens.size()); // 18 keywords + EOF
    
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
    
    return 0;
}

int test_lexer_all_operators() {
    std::string code = "+ - * / % == != < > <= >= && || ! = ++";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(16, tokens.size()); // 15 operators + EOF
    
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
    
    return 0;
}

int test_lexer_all_punctuation() {
    std::string code = "; , . : ( ) { } [ ]";
    ris::Lexer lexer(code);
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
    
    return 0;
}

int test_lexer_numeric_literals() {
    std::string code = "42 3.14";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(3, tokens.size()); // 2 literals + EOF
    
    // Test integer literals
    ASSERT_EQ(ris::TokenType::INTEGER_LITERAL, tokens[0].type);
    ASSERT_EQ("42", tokens[0].value);
    
    // Test float literals
    ASSERT_EQ(ris::TokenType::FLOAT_LITERAL, tokens[1].type);
    ASSERT_EQ("3.14", tokens[1].value);
    
    return 0;
}

int test_lexer_char_literals() {
    std::string code = "'a'";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(2, tokens.size()); // 1 char literal + EOF
    
    ASSERT_EQ(ris::TokenType::CHAR_LITERAL, tokens[0].type);
    ASSERT_EQ("'a'", tokens[0].value);
    
    return 0;
}

int test_lexer_string_literals_complex() {
    std::string code = "\"hello\"";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(2, tokens.size()); // 1 string literal + EOF
    
    ASSERT_EQ(ris::TokenType::STRING_LITERAL, tokens[0].type);
    ASSERT_EQ("\"hello\"", tokens[0].value);
    
    return 0;
}

int test_lexer_identifiers_complex() {
    std::string code = "variable_name _underscore camelCase PascalCase var123 _123abc";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(7, tokens.size()); // 6 identifiers + EOF
    
    for (size_t i = 0; i < 6; ++i) {
        ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[i].type);
    }
    
    ASSERT_EQ("variable_name", tokens[0].value);
    ASSERT_EQ("_underscore", tokens[1].value);
    ASSERT_EQ("camelCase", tokens[2].value);
    ASSERT_EQ("PascalCase", tokens[3].value);
    ASSERT_EQ("var123", tokens[4].value);
    ASSERT_EQ("_123abc", tokens[5].value);
    
    return 0;
}

int test_lexer_preprocessor_directives() {
    std::string code = "#include <std>";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(2, tokens.size()); // 1 system include + EOF
    
    ASSERT_EQ(ris::TokenType::SYSTEM_INCLUDE, tokens[0].type);
    
    return 0;
}

int test_lexer_comments_comprehensive() {
    std::string code = "int x = 5; // This is a comment\n/* This is a\n   multi-line comment */\nint y = 10;";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    // Should have: int, x, =, 5, ;, int, y, =, 10, ;, EOF
    ASSERT_EQ(11, tokens.size());
    
    ASSERT_EQ(ris::TokenType::INT, tokens[0].type);
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[1].type);
    ASSERT_EQ("x", tokens[1].value);
    ASSERT_EQ(ris::TokenType::ASSIGN, tokens[2].type);
    ASSERT_EQ(ris::TokenType::INTEGER_LITERAL, tokens[3].type);
    ASSERT_EQ("5", tokens[3].value);
    ASSERT_EQ(ris::TokenType::SEMICOLON, tokens[4].type);
    
    ASSERT_EQ(ris::TokenType::INT, tokens[5].type);
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[6].type);
    ASSERT_EQ("y", tokens[6].value);
    ASSERT_EQ(ris::TokenType::ASSIGN, tokens[7].type);
    ASSERT_EQ(ris::TokenType::INTEGER_LITERAL, tokens[8].type);
    ASSERT_EQ("10", tokens[8].value);
    ASSERT_EQ(ris::TokenType::SEMICOLON, tokens[9].type);
    
    return 0;
}

int test_lexer_whitespace_comprehensive() {
    std::string code = "int x = 5;";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(6, tokens.size()); // 5 tokens + EOF
    
    ASSERT_EQ(ris::TokenType::INT, tokens[0].type);
    ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[1].type);
    ASSERT_EQ("x", tokens[1].value);
    ASSERT_EQ(ris::TokenType::ASSIGN, tokens[2].type);
    ASSERT_EQ(ris::TokenType::INTEGER_LITERAL, tokens[3].type);
    ASSERT_EQ("5", tokens[3].value);
    ASSERT_EQ(ris::TokenType::SEMICOLON, tokens[4].type);
    
    return 0;
}

int test_lexer_error_conditions() {
    // Test unterminated string
    ris::Lexer lexer1("\"unterminated string");
    auto tokens1 = lexer1.tokenize();
    ASSERT_TRUE(lexer1.has_error());
    
    // Test invalid character
    ris::Lexer lexer3("int x = 5 @ invalid");
    auto tokens3 = lexer3.tokenize();
    ASSERT_TRUE(lexer3.has_error());
    
    return 0;
}

int test_lexer_position_tracking() {
    std::string code = "int x = 5;\nfloat y = 3.14;";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    // Test that positions are tracked correctly
    ASSERT_EQ(1, tokens[0].position.line); // int
    ASSERT_EQ(1, tokens[0].position.column);
    
    ASSERT_EQ(1, tokens[1].position.line); // x
    ASSERT_EQ(5, tokens[1].position.column);
    
    ASSERT_EQ(1, tokens[4].position.line); // 5
    ASSERT_EQ(9, tokens[4].position.column);
    
    ASSERT_EQ(2, tokens[6].position.line); // float
    ASSERT_EQ(1, tokens[6].position.column);
    
    return 0;
}

int test_lexer_edge_cases() {
    // Empty input
    ris::Lexer lexer1("");
    auto tokens1 = lexer1.tokenize();
    ASSERT_EQ(1, tokens1.size()); // Just EOF
    ASSERT_EQ(ris::TokenType::EOF_TOKEN, tokens1[0].type);
    
    // Only whitespace
    ris::Lexer lexer2("   \t\n\r   ");
    auto tokens2 = lexer2.tokenize();
    ASSERT_EQ(1, tokens2.size()); // Just EOF
    
    // Only comments
    ris::Lexer lexer3("// comment only\n/* another comment */");
    auto tokens3 = lexer3.tokenize();
    ASSERT_EQ(1, tokens3.size()); // Just EOF
    
    return 0;
}

int test_lexer_unicode_identifiers() {
    std::string code = "variable_alpha beta_gamma delta123";
    ris::Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(4, tokens.size()); // 3 identifiers + EOF
    
    for (size_t i = 0; i < 3; ++i) {
        ASSERT_EQ(ris::TokenType::IDENTIFIER, tokens[i].type);
    }
    
    ASSERT_EQ("variable_alpha", tokens[0].value);
    ASSERT_EQ("beta_gamma", tokens[1].value);
    ASSERT_EQ("delta123", tokens[2].value);
    
    return 0;
}

// Test functions are defined above, main() is in test_runner.cpp
