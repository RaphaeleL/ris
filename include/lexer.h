#pragma once

#include "token.h"
#include <string>
#include <vector>

namespace ris {

class Lexer {
public:
    explicit Lexer(const std::string& source);
    
    // Get the next token
    Token next_token();
    
    // Peek at the next token without consuming it
    Token peek_token();
    
    // Get all tokens at once
    std::vector<Token> tokenize();
    
    // Check if we're at the end of input
    bool is_at_end() const;
    
    // Get current position
    SourcePos current_position() const;
    
    // Check if there were any errors
    bool has_error() const { return has_error_; }
    const std::string& error_message() const { return error_message_; }

private:
    std::string source_;
    size_t current_pos_;
    size_t current_line_;
    size_t current_column_;
    bool has_error_;
    std::string error_message_;
    
    // Helper methods
    char current_char() const;
    char peek_char() const;
    char advance();
    void skip_whitespace();
    void skip_comment();
    
    // Token recognition methods
    Token scan_identifier();
    Token scan_number();
    Token scan_char_literal();
    Token scan_string_literal();
    Token scan_operator();
    Token scan_punctuation();
    
    // Character classification
    bool is_alpha(char c) const;
    bool is_digit(char c) const;
    bool is_alnum(char c) const;
    bool is_whitespace(char c) const;
};

} // namespace ris
