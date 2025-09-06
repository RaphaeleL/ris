#include "runtime.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

extern "C" {

// Generic print function (like Python's print)
void print(type_tag_t type, const void* value) {
    switch (type) {
        case TYPE_INT:
            std::cout << *static_cast<const int64_t*>(value);
            break;
        case TYPE_FLOAT:
            std::cout << *static_cast<const double*>(value);
            break;
        case TYPE_BOOL:
            std::cout << (*static_cast<const int8_t*>(value) ? "true" : "false");
            break;
        case TYPE_CHAR:
            std::cout << static_cast<char>(*static_cast<const int8_t*>(value));
            break;
        case TYPE_STRING:
            if (value) {
                std::cout << static_cast<const char*>(value);
            }
            break;
        default:
            std::cout << "<unknown type>";
            break;
    }
}

void println(type_tag_t type, const void* value) {
    print(type, value);
    std::cout << std::endl;
}

void print_with_space(type_tag_t type, const void* value) {
    print(type, value);
    std::cout << " ";
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

