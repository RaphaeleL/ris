#pragma once

#include <string>
#include <string_view>

namespace ris {

// Token types for the RIS language
enum class TokenType {
    // End of file
    EOF_TOKEN,
    
    // Identifiers and literals
    IDENTIFIER,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    CHAR_LITERAL,
    STRING_LITERAL,
    
    // Keywords
    INT, FLOAT, BOOL, CHAR, STRING, VOID, LIST,
    IF, ELSE, WHILE, FOR, SWITCH, CASE, DEFAULT, BREAK, CONTINUE, RETURN,
    TRUE, FALSE,
    
    // Operators
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
    EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
    AND, OR, NOT,
    ASSIGN,
    INCREMENT,
    
    // Punctuation
    SEMICOLON, COMMA, DOT, COLON,
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET,
    
    // Preprocessor directives
    HASH,
    INCLUDE,
    SYSTEM_INCLUDE,
    
    // Special
    UNKNOWN
};

// Source position information
struct SourcePos {
    size_t line;
    size_t column;
    size_t offset;
    
    SourcePos() : line(1), column(1), offset(0) {}
    SourcePos(size_t l, size_t c, size_t o) : line(l), column(c), offset(o) {}
};

// Token representation
struct Token {
    TokenType type;
    std::string value;
    SourcePos position;
    
    Token() : type(TokenType::UNKNOWN) {}
    Token(TokenType t, std::string v, SourcePos pos) 
        : type(t), value(std::move(v)), position(pos) {}
    
    // Helper methods
    bool is_keyword() const;
    bool is_literal() const;
    bool is_operator() const;
    bool is_punctuation() const;
    
    // String representation for debugging
    std::string to_string() const;
};

// Helper functions
TokenType keyword_to_token_type(const std::string& keyword);
std::string token_type_to_string(TokenType type);

} // namespace ris
