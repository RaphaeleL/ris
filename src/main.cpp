#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <sys/wait.h>
#include <filesystem>
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "codegen.h"
#include "diagnostics.h"

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file;
    bool compile_executable = false;
    bool auto_run = false;
    bool output_specified = false;
    bool verbose = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            output_file = argv[i + 1];
            output_specified = true;
            // Check if output file doesn't have .ll extension (executable)
            if (output_file.find(".ll") == std::string::npos) {
                compile_executable = true;
            }
            i++; // Skip the next argument
        } else if (std::string(argv[i]) == "--run") {
            auto_run = true;
        } else if (std::string(argv[i]) == "--verbose") {
            verbose = true;
        } else if (input_file.empty() && std::string(argv[i]) != "-o" && std::string(argv[i]) != "--run" && std::string(argv[i]) != "--verbose") {
            // First non-flag argument is the input file
            input_file = argv[i];
        }
    }
    
    // Auto-derive output file name if not specified
    if (!output_specified) {
        if (auto_run) {
            // For --run, derive executable name from input file
            std::filesystem::path input_path(input_file);
            output_file = input_path.stem().string(); // Remove extension
            compile_executable = true;
        } else {
            // Default to "output" for LLVM IR
            output_file = "output";
        }
    }
    
    if (input_file.empty()) {
        std::cout << "Usage: " << argv[0] << " <input.c> [-o <output>] [--run] [--verbose]" << std::endl;
        std::cout << "  -o <output>   : Specify output name (optional, auto-derived for --run)" << std::endl;
        std::cout << "  --run         : Auto-run executable after compilation" << std::endl;
        std::cout << "  --verbose     : Show detailed compilation information" << std::endl;
        return 1;
    }
    
    if (verbose) {
        std::cout << "Input file: " << input_file << std::endl;
        std::cout << "Output file: " << output_file << std::endl;
    }
    
    // Read input file
    std::ifstream file(input_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open input file " << input_file << std::endl;
        return 1;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
    file.close();
    
    // Get the directory of the source file
    std::filesystem::path input_path(input_file);
    std::string source_dir = input_path.parent_path().string();
    if (source_dir.empty()) {
        source_dir = ".";
    }
    
    // Tokenize the source
    ris::Lexer lexer(source, source_dir);
    auto tokens = lexer.tokenize();
    
    if (lexer.has_error()) {
        std::cerr << "Lexer error: " << lexer.error_message() << std::endl;
        return 1;
    }
    
    if (verbose) {
        std::cout << "Tokenized " << tokens.size() << " tokens" << std::endl;
    }
    
    // Parse the tokens into AST
    ris::Parser parser(tokens);
    auto program = parser.parse();
    
    if (parser.has_error()) {
        std::cerr << "Parser error: " << parser.error_message() << std::endl;
        return 1;
    }
    
    if (verbose) {
        std::cout << "Parsed successfully!" << std::endl;
        std::cout << "Functions: " << program->functions.size() << std::endl;
        std::cout << "Global variables: " << program->globals.size() << std::endl;
    }
    
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
    
    if (verbose) {
        std::cout << "Semantic analysis passed!" << std::endl;
    }

    // Generate LLVM IR
    std::string llvm_output = compile_executable ? "out/temp_output.ll" : "out/" + output_file;
    
    // Ensure out directory exists for LLVM IR generation
    std::filesystem::create_directories("out");
    
    ris::CodeGenerator codegen;
    bool codegen_ok = codegen.generate(std::move(program), llvm_output);
    
    if (!codegen_ok) {
        std::cerr << "Code generation failed: " << codegen.error_message() << std::endl;
        return 1;
    }
    
    if (compile_executable) {
        if (verbose) {
            std::cout << "Code generation completed! Compiling to executable..." << std::endl;
        }
        
        // Ensure out directory exists
        std::filesystem::create_directories("out");
        
        // Use LLVM toolchain: llc -> clang for linking
        std::string asm_output = "out/temp_output.s";
        std::string runtime_lib = "runtime/libris_runtime.a";
        std::string final_output = output_file;
        
        #ifdef _WIN32
            final_output += ".exe";
        #endif
        
        // Step 1: Use llc to generate assembly from LLVM IR
        std::string llc_cmd = "llc -o " + asm_output + " " + llvm_output;
        
        if (verbose) {
            std::cout << "Running: " << llc_cmd << std::endl;
        }
        
        int llc_result = std::system(llc_cmd.c_str());
        if (llc_result != 0) {
            std::cerr << "Error: llc failed to generate assembly (exit code " << llc_result << ")" << std::endl;
            std::remove(llvm_output.c_str());
            return 1;
        }
        
        // Step 2: Use clang to link assembly with runtime library
        std::string link_cmd = "clang++ -o " + final_output + " " + asm_output + " " + runtime_lib;
        
        if (verbose) {
            std::cout << "Running: " << link_cmd << std::endl;
        }
        
        int link_result = std::system(link_cmd.c_str());
        if (link_result != 0) {
            std::cerr << "Error: clang linking failed (exit code " << link_result << ")" << std::endl;
            std::remove(llvm_output.c_str());
            std::remove(asm_output.c_str());
            return 1;
        }
        
        // Clean up temporary files
        std::remove(llvm_output.c_str());
        std::remove(asm_output.c_str());
        
        if (verbose) {
            std::cout << "Executable created: " << output_file << std::endl;
        }
        
        if (auto_run) {
            if (verbose) {
                std::cout << "Auto-running executable..." << std::endl;
                std::cout << "--- Output ---" << std::endl;
            }
            
            // Run the executable
            std::string run_cmd = "./" + final_output;
            int run_result = std::system(run_cmd.c_str());
            
            // Extract the actual exit code from system() result
            // On Unix systems, system() returns the exit code in the high byte
            int exit_code = WEXITSTATUS(run_result);
            
            if (verbose) {
                std::cout << "--- End Output ---" << std::endl;
                std::cout << "Executable exited with code: " << exit_code << std::endl;
            }
            
            // Exit with the same code as the executed program
            return exit_code;
        } else {
            if (verbose) {
                std::cout << "Run with: ./" << final_output << std::endl;
            }
        }
    } else {
        if (verbose) {
            std::cout << "Code generation completed! Output written to " << output_file << std::endl;
        }
    }

    return 0;
}
