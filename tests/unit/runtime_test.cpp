#include <iostream>
#include <cassert>
#include <cstring>
#include "std.h"

// Simple test framework
#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            std::cerr << " FAIL  " << #expected << " != " << #actual << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while(0)

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << " FAIL  " << #condition << " is false at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            std::cerr << " FAIL  " << #condition << " is true at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } \
    } while(0)

int test_runtime_list_creation() {
    std::cout << "Running test_runtime_list_creation .........";
    
    ris_list_t* list = ris_list_create(TYPE_INT, 4);
    ASSERT_TRUE(list != nullptr);
    ASSERT_EQ(0, list->size);
    ASSERT_EQ(4, list->capacity);
    
    ris_list_free(list);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_runtime_list_push_pop() {
    std::cout << "Running test_runtime_list_push_pop .........";
    
    ris_list_t* list = ris_list_create(TYPE_INT, 4);
    
    // Test push operations
    int value1 = 42;
    ris_list_push(list, &value1);
    ASSERT_EQ(1, list->size);
    
    int value2 = 84;
    ris_list_push(list, &value2);
    ASSERT_EQ(2, list->size);
    
    // Test pop operations
    void* popped_ptr = ris_list_pop(list);
    ASSERT_TRUE(popped_ptr != nullptr);
    int popped_value = *(int*)popped_ptr;
    ASSERT_EQ(84, popped_value);
    ASSERT_EQ(1, list->size);
    
    popped_ptr = ris_list_pop(list);
    ASSERT_TRUE(popped_ptr != nullptr);
    popped_value = *(int*)popped_ptr;
    ASSERT_EQ(42, popped_value);
    ASSERT_EQ(0, list->size);
    
    ris_list_free(list);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_runtime_list_get() {
    std::cout << "Running test_runtime_list_get .........";
    
    ris_list_t* list = ris_list_create(TYPE_INT, 4);
    
    // Add some values
    int values[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        ris_list_push(list, &values[i]);
    }
    
    // Test get operations
    void* retrieved_ptr = ris_list_get(list, 0);
    ASSERT_TRUE(retrieved_ptr != nullptr);
    int retrieved_value = *(int*)retrieved_ptr;
    ASSERT_EQ(10, retrieved_value);
    
    retrieved_ptr = ris_list_get(list, 2);
    ASSERT_TRUE(retrieved_ptr != nullptr);
    retrieved_value = *(int*)retrieved_ptr;
    ASSERT_EQ(30, retrieved_value);
    
    retrieved_ptr = ris_list_get(list, 4);
    ASSERT_TRUE(retrieved_ptr != nullptr);
    retrieved_value = *(int*)retrieved_ptr;
    ASSERT_EQ(50, retrieved_value);
    
    ris_list_free(list);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_runtime_list_size() {
    std::cout << "Running test_runtime_list_size .........";
    
    ris_list_t* list = ris_list_create(TYPE_INT, 4);
    
    ASSERT_EQ(0, ris_list_size(list));
    
    // Add elements
    for (int i = 0; i < 10; i++) {
        ris_list_push(list, &i);
    }
    
    ASSERT_EQ(10, ris_list_size(list));
    
    ris_list_free(list);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_runtime_list_different_types() {
    std::cout << "Running test_runtime_list_different_types .........";
    
    // Test with float
    ris_list_t* float_list = ris_list_create(TYPE_FLOAT, 4);
    float f1 = 3.14f, f2 = 2.71f;
    ris_list_push(float_list, &f1);
    ris_list_push(float_list, &f2);
    
    void* retrieved_ptr = ris_list_get(float_list, 0);
    ASSERT_TRUE(retrieved_ptr != nullptr);
    float retrieved_float = *(float*)retrieved_ptr;
    ASSERT_TRUE(retrieved_float > 3.13f && retrieved_float < 3.15f);
    
    ris_list_free(float_list);
    
    // Test with char
    ris_list_t* char_list = ris_list_create(TYPE_CHAR, 4);
    char c1 = 'A', c2 = 'B';
    ris_list_push(char_list, &c1);
    ris_list_push(char_list, &c2);
    
    retrieved_ptr = ris_list_get(char_list, 0);
    ASSERT_TRUE(retrieved_ptr != nullptr);
    char retrieved_char = *(char*)retrieved_ptr;
    ASSERT_EQ('A', retrieved_char);
    
    ris_list_free(char_list);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_runtime_list_bounds_checking() {
    std::cout << "Running test_runtime_list_bounds_checking .........";
    
    ris_list_t* list = ris_list_create(TYPE_INT, 4);
    
    // Test get with empty list
    void* ptr = ris_list_get(list, 0);
    ASSERT_TRUE(ptr == nullptr);
    
    // Add one element
    int test_value = 42;
    ris_list_push(list, &test_value);
    
    // Test valid access
    ptr = ris_list_get(list, 0);
    ASSERT_TRUE(ptr != nullptr);
    int value = *(int*)ptr;
    ASSERT_EQ(42, value);
    
    // Test out of bounds access
    ptr = ris_list_get(list, 1);
    ASSERT_TRUE(ptr == nullptr);
    
    ris_list_free(list);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_runtime_list_pop_empty() {
    std::cout << "Running test_runtime_list_pop_empty .........";
    
    ris_list_t* list = ris_list_create(TYPE_INT, 4);
    
    // Test pop from empty list
    void* ptr = ris_list_pop(list);
    ASSERT_TRUE(ptr == nullptr);
    ASSERT_EQ(0, list->size);
    
    ris_list_free(list);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_runtime_memory_management() {
    std::cout << "Running test_runtime_memory_management .........";
    
    // Test multiple list creation and destruction
    ris_list_t* lists[10];
    for (int i = 0; i < 10; i++) {
        lists[i] = ris_list_create(TYPE_INT, 4);
        ASSERT_TRUE(lists[i] != nullptr);
        
        // Add some data to each list
        for (int j = 0; j < 5; j++) {
            ris_list_push(lists[i], &j);
        }
    }
    
    // Verify all lists are independent
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(5, ris_list_size(lists[i]));
    }
    
    // Destroy all lists
    for (int i = 0; i < 10; i++) {
        ris_list_free(lists[i]);
    }
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_runtime_list_resize() {
    std::cout << "Running test_runtime_list_resize .........";
    
    ris_list_t* list = ris_list_create(TYPE_INT, 4);
    
    // Add many elements to trigger resize
    for (int i = 0; i < 100; i++) {
        ris_list_push(list, &i);
    }
    
    ASSERT_EQ(100, ris_list_size(list));
    ASSERT_TRUE(list->capacity >= 100);
    
    // Verify all elements are correct
    for (int i = 0; i < 100; i++) {
        void* ptr = ris_list_get(list, i);
        ASSERT_TRUE(ptr != nullptr);
        int value = *(int*)ptr;
        ASSERT_EQ(i, value);
    }
    
    ris_list_free(list);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_runtime_print_functions() {
    std::cout << "Running test_runtime_print_functions .........";
    
    // Test that print functions exist and can be called
    // Note: We can't easily test output, but we can test that they don't crash
    
    // Test print with different types
    int int_val = 42;
    print(TYPE_INT, &int_val);
    
    float float_val = 3.14f;
    print(TYPE_FLOAT, &float_val);
    
    int8_t bool_val = 1;
    print(TYPE_BOOL, &bool_val);
    
    int8_t char_val = 'A';
    print(TYPE_CHAR, &char_val);
    
    const char* string_val = "Hello World";
    print(TYPE_STRING, string_val);
    
    // Test println
    println(TYPE_INT, &int_val);
    println(TYPE_FLOAT, &float_val);
    println(TYPE_BOOL, &bool_val);
    println(TYPE_CHAR, &char_val);
    println(TYPE_STRING, string_val);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_runtime_string_operations() {
    std::cout << "Running test_runtime_string_operations .........";
    
    // Test string concatenation
    const char* str1 = "Hello";
    const char* str2 = "World";
    
    char* combined = ris_string_concat(str1, str2);
    ASSERT_TRUE(combined != nullptr);
    ASSERT_EQ(0, strcmp("HelloWorld", combined));
    
    // Test string length
    size_t length = ris_string_length(combined);
    ASSERT_EQ(10, length);
    
    // Cleanup
    free(combined);
    
    std::cout << " OK" << std::endl;
    return 0;
}

int test_runtime_memory_allocation() {
    std::cout << "Running test_runtime_memory_allocation .........";
    
    // Test malloc
    void* ptr1 = ris_malloc(100);
    ASSERT_TRUE(ptr1 != nullptr);
    
    // Test free
    ris_free(ptr1);
    
    // Test multiple allocations
    void* ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = ris_malloc(50);
        ASSERT_TRUE(ptrs[i] != nullptr);
    }
    
    // Free all
    for (int i = 0; i < 10; i++) {
        ris_free(ptrs[i]);
    }
    
    std::cout << " OK" << std::endl;
    return 0;
}