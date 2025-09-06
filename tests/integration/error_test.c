#include <std>
int main() {
    // Test undefined variable error
    int x = y;  // y is not defined
    
    // Test type mismatch error
    int a = "hello";  // string assigned to int
    
    // Test undefined function error
    unknown_function();
    
    return 0;
}
