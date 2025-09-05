#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "codegen.h"

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
    
    std::cout << "Tokenized " << tokens.size() << " tokens" << std::endl;
    
    // Parse the tokens into AST
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    if (parser.has_error()) {
        std::cerr << "Parser error: " << parser.error_message() << std::endl;
        return 1;
    }
    
    std::cout << "Parsed successfully!" << std::endl;
    std::cout << "Functions: " << program->functions.size() << std::endl;
    std::cout << "Global variables: " << program->globals.size() << std::endl;
    
    // Perform semantic analysis
    ris::SemanticAnalyzer analyzer;
    bool semantic_ok = analyzer.analyze(*program);
    
    if (!semantic_ok) {
        std::cerr << "Semantic analysis failed:" << std::endl;
        for (const auto& error : analyzer.errors()) {
            std::cerr << "  " << error << std::endl;
        }
        return 1;
    }
    
    std::cout << "Semantic analysis passed!" << std::endl;

    // Generate LLVM IR
    ris::CodeGenerator codegen;
    bool codegen_ok = codegen.generate(std::move(program), "output.ll");
    
    if (!codegen_ok) {
        std::cerr << "Code generation failed: " << codegen.error_message() << std::endl;
        return 1;
    }
    
    std::cout << "Code generation completed! Output written to output.ll" << std::endl;

    return 0;
}
