int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    
    int a = 0;
    int b = 1;
    int result = 0;
    
    for (int i = 2; i <= n; i = i + 1) {
        result = a + b;
        a = b;
        b = result;
    }
    
    return result;
}

int main() {
    println("Fibonacci sequence:");
    
    for (int i = 0; i < 10; i = i + 1) {
        int fib = fibonacci(i);
        println("fib(", i, ") = ", fib);
    }
    
    // Test specific values
    int fib_5 = fibonacci(5);
    int fib_10 = fibonacci(10);
    
    println("fib(5) = ", fib_5);
    println("fib(10) = ", fib_10);
    
    return 0;
}
