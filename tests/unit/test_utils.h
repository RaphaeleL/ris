#pragma once

#include <iostream>
#include <string>
#include <vector>

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m"

namespace test_utils {

// Calculate the maximum test name length from a list of test names
size_t get_max_test_name_length(const std::vector<std::string>& test_names);

// Print a test name with properly aligned dots
void print_test_start(const std::string& test_name, size_t max_length);

// Print test success
void print_test_success();

// Print test failure with message
void print_test_failure(const std::string& message);

// Global variable to store max length (set by test runner)
extern size_t g_max_test_name_length;

} // namespace test_utils

// Macro for test functions to use
#define PRINT_TEST_START(name) test_utils::print_test_start(name, test_utils::g_max_test_name_length)
#define PRINT_TEST_SUCCESS() test_utils::print_test_success()
#define PRINT_TEST_FAILURE(msg) test_utils::print_test_failure(msg)
