// Test multi-argument print functions
int main() {
    // Test print with multiple arguments
    println("Hello", " ", "World", "!");
    println("Numbers: ", 42, " and ", 3.14);
    println("Boolean: ", true, " and ", false);
    
    // Test println with multiple arguments
    println("Multi-arg", " ", "println", " ", "works!");
    println("Mixed types:", 100, 2.5, true, 'X');
    println("Empty println:");
    println();
    
    return 0;
}
