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
        case TYPE_LIST:
            if (value) {
                const ris_list_t* list = static_cast<const ris_list_t*>(value);
                std::cout << "[";
                for (size_t i = 0; i < list->size; ++i) {
                    if (i > 0) {
                        std::cout << ", ";
                    }
                    // Print each element using the element type
                    if (list->element_type == TYPE_LIST) {
                        // For nested lists, the data[i] is a pointer to another ris_list_t*
                        const ris_list_t* nested_list = static_cast<const ris_list_t*>(list->data[i]);
                        print(TYPE_LIST, nested_list);
                    } else {
                        print(list->element_type, list->data[i]);
                    }
                }
                std::cout << "]";
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


void ris_exit(int32_t code) {
    std::exit(code);
}

// List functions
ris_list_t* ris_list_create(type_tag_t element_type, size_t initial_capacity) {
    ris_list_t* list = static_cast<ris_list_t*>(std::malloc(sizeof(ris_list_t)));
    if (!list) return nullptr;
    
    list->data = static_cast<void**>(std::malloc(initial_capacity * sizeof(void*)));
    if (!list->data) {
        std::free(list);
        return nullptr;
    }
    
    list->size = 0;
    list->capacity = initial_capacity;
    list->element_type = element_type;
    return list;
}

void ris_list_free(ris_list_t* list) {
    if (!list) return;
    
    // Free all elements
    for (size_t i = 0; i < list->size; ++i) {
        if (list->data[i]) {
            std::free(list->data[i]);
        }
    }
    
    std::free(list->data);
    std::free(list);
}

void ris_list_push(ris_list_t* list, void* element) {
    if (!list) return;
    
    // Resize if needed
    if (list->size >= list->capacity) {
        size_t new_capacity = list->capacity * 2;
        void** new_data = static_cast<void**>(std::realloc(list->data, new_capacity * sizeof(void*)));
        if (!new_data) return; // Out of memory
        list->data = new_data;
        list->capacity = new_capacity;
    }
    
    list->data[list->size] = element;
    list->size++;
}

void* ris_list_pop(ris_list_t* list) {
    if (!list || list->size == 0) return nullptr;
    
    list->size--;
    return list->data[list->size];
}

size_t ris_list_size(ris_list_t* list) {
    return list ? list->size : 0;
}

void* ris_list_get(ris_list_t* list, size_t index) {
    if (!list || index >= list->size) return nullptr;
    return list->data[index];
}

// Specific getter for nested lists that returns a ris_list_t* pointer
ris_list_t* ris_list_get_list(ris_list_t* list, size_t index) {
    if (!list || index >= list->size) return nullptr;
    return static_cast<ris_list_t*>(list->data[index]);
}

// Helper functions for list element access
int64_t ris_list_get_int(ris_list_t* list, size_t index) {
    if (!list || index >= list->size || list->element_type != TYPE_INT) return 0;
    return *static_cast<int64_t*>(list->data[index]);
}

double ris_list_get_float(ris_list_t* list, size_t index) {
    if (!list || index >= list->size || list->element_type != TYPE_FLOAT) return 0.0;
    return *static_cast<double*>(list->data[index]);
}

int8_t ris_list_get_bool(ris_list_t* list, size_t index) {
    if (!list || index >= list->size || list->element_type != TYPE_BOOL) return 0;
    return *static_cast<int8_t*>(list->data[index]);
}

int8_t ris_list_get_char(ris_list_t* list, size_t index) {
    if (!list || index >= list->size || list->element_type != TYPE_CHAR) return 0;
    return *static_cast<int8_t*>(list->data[index]);
}

const char* ris_list_get_string(ris_list_t* list, size_t index) {
    if (!list || index >= list->size || list->element_type != TYPE_STRING) return nullptr;
    return static_cast<const char*>(list->data[index]);
}

} // extern "C"

