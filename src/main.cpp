#include <iostream>
#include <string>
#include <vector>

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
    
    // TODO: Implement actual compilation pipeline
    std::cout << "Compilation not yet implemented." << std::endl;
    
    return 0;
}
