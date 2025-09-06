#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

// Runtime functions for the RIS language
extern "C" {

// Type tags for generic print function
typedef enum {
    TYPE_INT = 0,
    TYPE_FLOAT = 1,
    TYPE_BOOL = 2,
    TYPE_CHAR = 3,
    TYPE_STRING = 4
} type_tag_t;

// Print functions (like Python's print)
void print(type_tag_t type, const void* value);
void println(type_tag_t type, const void* value);
void print_with_space(type_tag_t type, const void* value);

// Memory management functions
void* ris_malloc(size_t size);
void ris_free(void* ptr);

// String functions
char* ris_string_concat(const char* str1, const char* str2);
size_t ris_string_length(const char* str);

// Array functions
void* ris_array_alloc(size_t element_size, size_t count);
void ris_array_free(void* ptr);

// Utility functions
void ris_exit(int32_t code);

} // extern "C"

