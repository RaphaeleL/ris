#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

// Runtime functions for the RIS language
extern "C" {

// Basic I/O functions
void ris_print_int(int64_t value);
void ris_print_float(double value);
void ris_print_bool(int8_t value);
void ris_print_char(int8_t value);
void ris_print_string(const char* str);
void ris_println_int(int64_t value);
void ris_println_float(double value);
void ris_println_bool(int8_t value);
void ris_println_char(int8_t value);
void ris_println_string(const char* str);
void ris_println();

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
