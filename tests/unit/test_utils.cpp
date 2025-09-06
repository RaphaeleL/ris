#include "test_utils.h"

namespace test_utils {

// Global variable to store max length
size_t g_max_test_name_length = 0;

size_t get_max_test_name_length(const std::vector<std::string>& test_names) {
    size_t max_length = 0;
    for (const auto& name : test_names) {
        max_length = std::max(max_length, name.length());
    }
    return max_length;
}

void print_test_start(const std::string& test_name, size_t max_length) {
    std::cout << "Running " << test_name;
    
    // Calculate how many dots we need to reach the target length
    size_t current_length = 8 + test_name.length(); // "Running " + test_name
    size_t target_length = max_length + 8 + 5; // "Running " + max_name + padding
    size_t dots_needed = (target_length > current_length) ? (target_length - current_length) : 0;
    
    for (size_t i = 0; i < dots_needed; ++i) {
        std::cout << ".";
    }
}

void print_test_success() {
    std::cout << " OK" << std::endl;
}

void print_test_failure(const std::string& message) {
    std::cout << " FAIL  " << message << std::endl;
}

} // namespace test_utils
