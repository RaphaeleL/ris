#include "lexer.h"
#include <cctype>
#include <fstream>
#include <filesystem>

namespace ris {

Lexer::Lexer(const std::string& source, const std::string& source_dir) 
    : source_(source), source_dir_(source_dir), current_pos_(0), current_line_(1), current_column_(1), 
      has_error_(false), error_message_("") {
}

Token Lexer::next_token() {
    skip_whitespace();
    
    // Check for comments after skipping whitespace
    if (!is_at_end() && current_char() == '/' && peek_char() == '/') {
        skip_comment();
        return next_token(); // recursively get next token after comment
    }
    if (!is_at_end() && current_char() == '/' && peek_char() == '*') {
        skip_comment();
        return next_token(); // recursively get next token after comment
    }
    
    if (is_at_end()) {
        return Token(TokenType::EOF_TOKEN, "", current_position());
    }
    
    char c = current_char();
    
    // Identifiers and keywords
    if (is_alpha(c) || c == '_') {
        return scan_identifier();
    }
    
    // Numbers
    if (is_digit(c)) {
        return scan_number();
    }
    
    // Character literals
    if (c == '\'') {
        return scan_char_literal();
    }
    
    // String literals
    if (c == '"') {
        return scan_string_literal();
    }
    
    // Operators and punctuation
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
        c == '=' || c == '!' || c == '<' || c == '>' ||
        c == '&' || c == '|') {
        return scan_operator();
    }
    
    // Punctuation
    if (c == ';' || c == ',' || c == '.' ||
        c == '(' || c == ')' || c == '{' || c == '}' ||
        c == '[' || c == ']') {
        return scan_punctuation();
    }
    
    // Preprocessor directives
    if (c == '#') {
        return scan_preprocessor();
    }
    
    // Unknown character
    Token token(TokenType::UNKNOWN, std::string(1, c), current_position());
    advance();
    return token;
}

Token Lexer::peek_token() {
    size_t saved_pos = current_pos_;
    size_t saved_line = current_line_;
    size_t saved_column = current_column_;
    
    Token token = next_token();
    
    current_pos_ = saved_pos;
    current_line_ = saved_line;
    current_column_ = saved_column;
    
    return token;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!is_at_end()) {
        Token token = next_token();
        
        if (token.type == TokenType::INCLUDE) {
            // Read the included file and tokenize it
            std::string filename = token.value;
            std::string include_content = read_include_file(filename, source_dir_);
            
            if (has_error_) {
                // Error reading include file, return empty tokens
                return {};
            }
            
            // Create a new lexer for the included content
            Lexer include_lexer(include_content, source_dir_);
            auto include_tokens = include_lexer.tokenize();
            
            if (include_lexer.has_error()) {
                has_error_ = true;
                error_message_ = "Error in included file " + filename + ": " + include_lexer.error_message();
                return {};
            }
            
            // Add the included tokens (excluding EOF)
            for (const auto& include_token : include_tokens) {
                if (include_token.type != TokenType::EOF_TOKEN) {
                    tokens.push_back(include_token);
                }
            }
        } else {
            tokens.push_back(token);
            
            if (token.type == TokenType::EOF_TOKEN) {
                break;
            }
        }
    }
    
    // Ensure we always have an EOF token at the end
    if (tokens.empty() || tokens.back().type != TokenType::EOF_TOKEN) {
        tokens.push_back(Token(TokenType::EOF_TOKEN, "", current_position()));
    }
    
    return tokens;
}

bool Lexer::is_at_end() const {
    return current_pos_ >= source_.length();
}

SourcePos Lexer::current_position() const {
    return SourcePos(current_line_, current_column_, current_pos_);
}

char Lexer::current_char() const {
    return is_at_end() ? '\0' : source_[current_pos_];
}

char Lexer::peek_char() const {
    return (current_pos_ + 1 >= source_.length()) ? '\0' : source_[current_pos_ + 1];
}

char Lexer::advance() {
    if (is_at_end()) {
        return '\0';
    }
    
    char c = source_[current_pos_++];
    
    if (c == '\n') {
        current_line_++;
        current_column_ = 1;
    } else {
        current_column_++;
    }
    
    return c;
}

void Lexer::skip_whitespace() {
    while (!is_at_end() && is_whitespace(current_char())) {
        advance();
    }
}

void Lexer::skip_comment() {
    if (current_char() == '/' && peek_char() == '/') {
        // Single-line comment
        while (!is_at_end() && current_char() != '\n') {
            advance();
        }
    } else if (current_char() == '/' && peek_char() == '*') {
        // Multi-line comment
        advance(); // consume '/'
        advance(); // consume '*'
        
        while (!is_at_end()) {
            if (current_char() == '*' && peek_char() == '/') {
                advance(); // consume '*'
                advance(); // consume '/'
                break;
            }
            advance();
        }
    }
}

Token Lexer::scan_identifier() {
    SourcePos start_pos = current_position();
    std::string value;
    
    while (!is_at_end() && is_alnum(current_char())) {
        value += advance();
    }
    
    TokenType type = keyword_to_token_type(value);
    return Token(type, value, start_pos);
}

Token Lexer::scan_number() {
    SourcePos start_pos = current_position();
    std::string value;
    
    // Integer part
    while (!is_at_end() && is_digit(current_char())) {
        value += advance();
    }
    
    // Check for float
    if (!is_at_end() && current_char() == '.' && is_digit(peek_char())) {
        value += advance(); // consume '.'
        while (!is_at_end() && is_digit(current_char())) {
            value += advance();
        }
        return Token(TokenType::FLOAT_LITERAL, value, start_pos);
    }
    
    return Token(TokenType::INTEGER_LITERAL, value, start_pos);
}

Token Lexer::scan_char_literal() {
    SourcePos start_pos = current_position();
    advance(); // consume opening quote
    
    std::string value;
    if (!is_at_end() && current_char() != '\'') {
        if (current_char() == '\\') {
            advance(); // consume backslash
            if (!is_at_end()) {
                char c = advance();
                switch (c) {
                    case 'n': value = "\n"; break;
                    case 't': value = "\t"; break;
                    case 'r': value = "\r"; break;
                    case '\\': value = "\\"; break;
                    case '\'': value = "'"; break;
                    case '"': value = "\""; break;
                    default: value = std::string(1, c); break;
                }
            }
        } else {
            value = advance();
        }
    }
    
    if (is_at_end() || current_char() != '\'') {
        has_error_ = true;
        error_message_ = "Unterminated character literal";
        return Token(TokenType::UNKNOWN, "", start_pos);
    }
    advance(); // consume closing quote
    
    return Token(TokenType::CHAR_LITERAL, value, start_pos);
}

Token Lexer::scan_string_literal() {
    SourcePos start_pos = current_position();
    advance(); // consume opening quote
    
    std::string value;
    while (!is_at_end() && current_char() != '"') {
        if (current_char() == '\\') {
            advance(); // consume backslash
            if (!is_at_end()) {
                char c = advance();
                switch (c) {
                    case 'n': value += "\n"; break;
                    case 't': value += "\t"; break;
                    case 'r': value += "\r"; break;
                    case '\\': value += "\\"; break;
                    case '\'': value += "'"; break;
                    case '"': value += "\""; break;
                    default: value += c; break;
                }
            }
        } else {
            value += advance();
        }
    }
    
    if (is_at_end()) {
        has_error_ = true;
        error_message_ = "Unterminated string literal";
        return Token(TokenType::UNKNOWN, "", start_pos);
    }
    advance(); // consume closing quote
    
    return Token(TokenType::STRING_LITERAL, value, start_pos);
}

Token Lexer::scan_operator() {
    SourcePos start_pos = current_position();
    char c = current_char();
    
    switch (c) {
        case '+':
            advance();
            return Token(TokenType::PLUS, "+", start_pos);
            
        case '-':
            advance();
            return Token(TokenType::MINUS, "-", start_pos);
            
        case '*':
            advance();
            return Token(TokenType::MULTIPLY, "*", start_pos);
            
        case '/':
            advance();
            return Token(TokenType::DIVIDE, "/", start_pos);
            
        case '%':
            advance();
            return Token(TokenType::MODULO, "%", start_pos);
            
        case '=':
            advance();
            if (!is_at_end() && current_char() == '=') {
                advance();
                return Token(TokenType::EQUAL, "==", start_pos);
            }
            return Token(TokenType::ASSIGN, "=", start_pos);
            
        case '!':
            advance();
            if (!is_at_end() && current_char() == '=') {
                advance();
                return Token(TokenType::NOT_EQUAL, "!=", start_pos);
            }
            return Token(TokenType::NOT, "!", start_pos);
            
        case '<':
            advance();
            if (!is_at_end() && current_char() == '=') {
                advance();
                return Token(TokenType::LESS_EQUAL, "<=", start_pos);
            }
            return Token(TokenType::LESS, "<", start_pos);
            
        case '>':
            advance();
            if (!is_at_end() && current_char() == '=') {
                advance();
                return Token(TokenType::GREATER_EQUAL, ">=", start_pos);
            }
            return Token(TokenType::GREATER, ">", start_pos);
            
        case '&':
            advance();
            if (!is_at_end() && current_char() == '&') {
                advance();
                return Token(TokenType::AND, "&&", start_pos);
            }
            has_error_ = true;
            error_message_ = "Unexpected '&' character";
            return Token(TokenType::UNKNOWN, "&", start_pos);
            
        case '|':
            advance();
            if (!is_at_end() && current_char() == '|') {
                advance();
                return Token(TokenType::OR, "||", start_pos);
            }
            has_error_ = true;
            error_message_ = "Unexpected '|' character";
            return Token(TokenType::UNKNOWN, "|", start_pos);
            
        default:
            has_error_ = true;
            error_message_ = "Unexpected character in operator scan";
            return Token(TokenType::UNKNOWN, std::string(1, c), start_pos);
    }
}

Token Lexer::scan_punctuation() {
    SourcePos start_pos = current_position();
    char c = current_char();
    
    switch (c) {
        case ';':
            advance();
            return Token(TokenType::SEMICOLON, ";", start_pos);
        case ',':
            advance();
            return Token(TokenType::COMMA, ",", start_pos);
        case '.':
            advance();
            return Token(TokenType::DOT, ".", start_pos);
        case '(':
            advance();
            return Token(TokenType::LEFT_PAREN, "(", start_pos);
        case ')':
            advance();
            return Token(TokenType::RIGHT_PAREN, ")", start_pos);
        case '{':
            advance();
            return Token(TokenType::LEFT_BRACE, "{", start_pos);
        case '}':
            advance();
            return Token(TokenType::RIGHT_BRACE, "}", start_pos);
        case '[':
            advance();
            return Token(TokenType::LEFT_BRACKET, "[", start_pos);
        case ']':
            advance();
            return Token(TokenType::RIGHT_BRACKET, "]", start_pos);
        default:
            has_error_ = true;
            error_message_ = "Unexpected character in punctuation scan";
            return Token(TokenType::UNKNOWN, std::string(1, c), start_pos);
    }
}

Token Lexer::scan_preprocessor() {
    SourcePos start_pos = current_position();
    advance(); // consume '#'
    
    // Skip whitespace after #
    skip_whitespace();
    
    // Read the directive (include, define, etc.)
    std::string directive;
    while (!is_at_end() && is_alpha(current_char())) {
        directive += advance();
    }
    
    if (directive == "include") {
        // Skip whitespace after 'include'
        skip_whitespace();
        
        // Read the filename (expecting quotes)
        if (current_char() == '"') {
            advance(); // consume opening quote
            std::string filename;
            while (!is_at_end() && current_char() != '"') {
                filename += advance();
            }
            if (is_at_end()) {
                has_error_ = true;
                error_message_ = "Unterminated include filename";
                return Token(TokenType::UNKNOWN, "", start_pos);
            }
            advance(); // consume closing quote
            
            // Skip to end of line
            while (!is_at_end() && current_char() != '\n') {
                advance();
            }
            
            return Token(TokenType::INCLUDE, filename, start_pos);
        } else {
            has_error_ = true;
            error_message_ = "Expected quoted filename after #include";
            return Token(TokenType::UNKNOWN, "", start_pos);
        }
    } else {
        has_error_ = true;
        error_message_ = "Unknown preprocessor directive: " + directive;
        return Token(TokenType::UNKNOWN, directive, start_pos);
    }
}

bool Lexer::is_alpha(char c) const {
    return std::isalpha(c) || c == '_';
}

bool Lexer::is_digit(char c) const {
    return std::isdigit(c);
}

bool Lexer::is_alnum(char c) const {
    return is_alpha(c) || is_digit(c);
}

bool Lexer::is_whitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

std::string Lexer::read_include_file(const std::string& filename, const std::string& current_dir) {
    std::string full_path;
    
    // If filename starts with /, it's an absolute path
    if (filename[0] == '/') {
        full_path = filename;
    } else {
        // Relative path - combine with current directory
        full_path = current_dir + "/" + filename;
    }
    
    std::ifstream file(full_path);
    if (!file.is_open()) {
        has_error_ = true;
        error_message_ = "Could not open include file: " + full_path;
        return "";
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    return content;
}

} // namespace ris
