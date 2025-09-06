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
    TYPE_STRING = 4,
    TYPE_LIST = 5
} type_tag_t;

// List structure for runtime
typedef struct {
    void** data;        // Array of pointers to elements
    size_t size;        // Number of elements
    size_t capacity;    // Allocated capacity
    type_tag_t element_type; // Type of elements in the list
} ris_list_t;

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

// List functions
ris_list_t* ris_list_create(type_tag_t element_type, size_t initial_capacity);
void ris_list_free(ris_list_t* list);
void ris_list_push(ris_list_t* list, void* element);
void* ris_list_pop(ris_list_t* list);
size_t ris_list_size(ris_list_t* list);
void* ris_list_get(ris_list_t* list, size_t index);
ris_list_t* ris_list_get_list(ris_list_t* list, size_t index);

// Helper functions for list element access
int64_t ris_list_get_int(ris_list_t* list, size_t index);
double ris_list_get_float(ris_list_t* list, size_t index);
int8_t ris_list_get_bool(ris_list_t* list, size_t index);
int8_t ris_list_get_char(ris_list_t* list, size_t index);
const char* ris_list_get_string(ris_list_t* list, size_t index);


// Utility functions
void ris_exit(int32_t code);

} // extern "C"

