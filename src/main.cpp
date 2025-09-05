#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "lexer.h"

int main(int argc, char* argv[]) {
    std::cout << "RIS Compiler v0.1.0" << std::endl;
    
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <input.c> [-o <output.ll>]" << std::endl;
        return 1;
    }
    
    std::string input_file = argv[1];
    std::string output_file = "output.ll";
    
    // Parse command line arguments
    for (int i = 2; i < argc; i++) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            output_file = argv[i + 1];
            i++; // Skip the next argument
        }
    }
    
    std::cout << "Input file: " << input_file << std::endl;
    std::cout << "Output file: " << output_file << std::endl;
    
    // Read input file
    std::ifstream file(input_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open input file " << input_file << std::endl;
        return 1;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
    file.close();
    
    // Tokenize the source
    ris::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    if (lexer.has_error()) {
        std::cerr << "Lexer error: " << lexer.error_message() << std::endl;
        return 1;
    }
    
    std::cout << "Tokenized " << tokens.size() << " tokens:" << std::endl;
    for (const auto& token : tokens) {
        if (token.type != ris::TokenType::EOF_TOKEN) {
            std::cout << "  " << token.to_string() << std::endl;
        }
    }
    
    // TODO: Implement parser, semantic analysis, and code generation
    
    return 0;
}
