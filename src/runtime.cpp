#include "runtime.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

extern "C" {

void ris_print_int(int64_t value) {
    std::cout << value;
}

void ris_print_float(double value) {
    std::cout << value;
}

void ris_print_bool(int8_t value) {
    std::cout << (value ? "true" : "false");
}

void ris_print_char(int8_t value) {
    std::cout << static_cast<char>(value);
}

void ris_print_string(const char* str) {
    if (str) {
        std::cout << str;
    }
}

void ris_println_int(int64_t value) {
    std::cout << value << std::endl;
}

void ris_println_float(double value) {
    std::cout << value << std::endl;
}

void ris_println_bool(int8_t value) {
    std::cout << (value ? "true" : "false") << std::endl;
}

void ris_println_char(int8_t value) {
    std::cout << static_cast<char>(value) << std::endl;
}

void ris_println_string(const char* str) {
    if (str) {
        std::cout << str << std::endl;
    } else {
        std::cout << std::endl;
    }
}

void ris_println() {
    std::cout << std::endl;
}

void* ris_malloc(size_t size) {
    return std::malloc(size);
}

void ris_free(void* ptr) {
    if (ptr) {
        std::free(ptr);
    }
}

char* ris_string_concat(const char* str1, const char* str2) {
    if (!str1) str1 = "";
    if (!str2) str2 = "";
    
    size_t len1 = std::strlen(str1);
    size_t len2 = std::strlen(str2);
    size_t total_len = len1 + len2 + 1;
    
    char* result = static_cast<char*>(std::malloc(total_len));
    if (result) {
        std::strcpy(result, str1);
        std::strcat(result, str2);
    }
    
    return result;
}

size_t ris_string_length(const char* str) {
    return str ? std::strlen(str) : 0;
}

void* ris_array_alloc(size_t element_size, size_t count) {
    return std::malloc(element_size * count);
}

void ris_array_free(void* ptr) {
    if (ptr) {
        std::free(ptr);
    }
}

void ris_exit(int32_t code) {
    std::exit(code);
}

} // extern "C"
