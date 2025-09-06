#include <std>
int main() {
    // Test string literals and printing
    println("Hello", "World");
    
    // Test string variables and concatenation
    string s1 = "Hello";
    string s2 = "World";
    string s3 = s1 + s2;
    println("Concatenated:", s3);
    
    // Test multiple concatenations
    string s4 = s1 + " " + s2;
    println("With space:", s4);
    
    // Test string + string literal
    string s5 = s1 + " Universe";
    println("String + literal:", s5);
    
    // Test string literal + string
    string s6 = "Hello " + s2;
    println("Literal + string:", s6);
    
    return 0;
}
