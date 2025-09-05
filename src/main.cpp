#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "codegen.h"

int main(int argc, char* argv[]) {
    std::cout << "RIS Compiler v0.1.0" << std::endl;
    
    std::string input_file;
    std::string output_file = "output.ll";
    bool compile_executable = false;
    bool auto_run = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            output_file = argv[i + 1];
            // Check if output file doesn't have .ll extension (executable)
            if (output_file.find(".ll") == std::string::npos) {
                compile_executable = true;
            }
            i++; // Skip the next argument
        } else if (std::string(argv[i]) == "--run") {
            auto_run = true;
        } else if (input_file.empty() && std::string(argv[i]) != "-o" && std::string(argv[i]) != "--run") {
            // First non-flag argument is the input file
            input_file = argv[i];
        }
    }
    
    if (input_file.empty()) {
        std::cout << "Usage: " << argv[0] << " <input.c> [-o <output>] [--run]" << std::endl;
        std::cout << "  -o <output>   : Generate executable" << std::endl;
        std::cout << "  --run         : Auto-run executable after compilation" << std::endl;
        return 1;
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
    std::string llvm_output = compile_executable ? "temp_output.ll" : output_file;
    ris::CodeGenerator codegen;
    bool codegen_ok = codegen.generate(std::move(program), llvm_output);
    
    if (!codegen_ok) {
        std::cerr << "Code generation failed: " << codegen.error_message() << std::endl;
        return 1;
    }
    
    if (compile_executable) {
        std::cout << "Code generation completed! Compiling to executable..." << std::endl;
        
        // Detect platform and try multiple compilers
        std::vector<std::string> compilers;
        std::string runtime_lib = "bin/libris_runtime.a";
        std::string output_ext = "";
        
        #ifdef __APPLE__
            // macOS - try clang++, clang, g++
            compilers = {"clang++", "clang", "g++"};
        #elif _WIN32
            // Windows - try clang++, clang, g++, cl
            compilers = {"clang++", "clang", "g++", "cl"};
            output_ext = ".exe";
        #else
            // Linux - try clang++, clang, g++
            compilers = {"clang++", "clang", "g++"};
        #endif
        
        std::string final_output = output_file + output_ext;
        bool compilation_success = false;
        
        for (const auto& compiler : compilers) {
            std::string compile_cmd;
            
            if (compiler == "cl") {
                // MSVC compiler
                compile_cmd = "cl /Fe:" + final_output + " " + llvm_output + " " + runtime_lib + " /link";
            } else {
                // GCC/Clang family
                compile_cmd = compiler + " -o " + final_output + " " + llvm_output + " " + runtime_lib;
            }
            
            std::cout << "Trying: " << compile_cmd << std::endl;
            
            // Execute compilation command
            int result = std::system(compile_cmd.c_str());
            
            if (result == 0) {
                std::cout << "Compilation successful with " << compiler << std::endl;
                compilation_success = true;
                break;
            } else {
                std::cout << "Failed with " << compiler << " (exit code " << result << ")" << std::endl;
            }
        }
        
        if (!compilation_success) {
            std::cerr << "Compilation failed with all available compilers!" << std::endl;
            std::cerr << "Tried: ";
            for (size_t i = 0; i < compilers.size(); ++i) {
                std::cerr << compilers[i];
                if (i < compilers.size() - 1) std::cerr << ", ";
            }
            std::cerr << std::endl;
            std::cerr << "Please install one of these compilers:" << std::endl;
            std::cerr << "  - clang++ (recommended)" << std::endl;
            std::cerr << "  - g++" << std::endl;
            #ifdef _WIN32
            std::cerr << "  - cl (Microsoft Visual C++)" << std::endl;
            #endif
            // Clean up temp file
            std::remove(llvm_output.c_str());
            return 1;
        }
        
        // Clean up temporary LLVM IR file
        std::remove(llvm_output.c_str());
        
        std::cout << "Executable created: " << output_file << std::endl;
        
        if (auto_run) {
            std::cout << "Auto-running executable..." << std::endl;
            std::cout << "--- Output ---" << std::endl;
            
            // Run the executable
            std::string run_cmd = "./" + final_output;
            int run_result = std::system(run_cmd.c_str());
            
            std::cout << "--- End Output ---" << std::endl;
            std::cout << "Executable exited with code: " << run_result << std::endl;
        } else {
            std::cout << "Run with: ./" << final_output << std::endl;
        }
    } else {
        std::cout << "Code generation completed! Output written to " << output_file << std::endl;
    }

    return 0;
}
