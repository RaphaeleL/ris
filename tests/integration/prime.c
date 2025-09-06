// Helper function to check if a number is prime
bool is_prime(int n) {
    if (n < 2) {
        return false;
    }
    if (n == 2) {
        return true;
    }
    
    // Check if n is even by seeing if n/2 * 2 equals n
    int half = n / 2;
    if (half * 2 == n) {
        return false;
    }
    
    // Check odd divisors from 3 to sqrt(n) approximation
    int i = 3;
    while (i * i <= n) {
        // Check if i divides n by seeing if (n/i) * i equals n
        int quotient = n / i;
        if (quotient * i == n) {
            return false;
        }
        i = i + 2;  // Only check odd numbers
    }
    
    return true;
}

// Function to find all primes up to a given limit
int count_primes(int limit) {
    int count = 0;
    
    for (int i = 2; i <= limit; i = i + 1) {
        if (is_prime(i)) {
            count = count + 1;
            println("Prime found: ", i);
        }
    }
    
    return count;
}

int main() {
    println("Prime number checker:");
    
    // Test individual numbers
    list<int> test_numbers = [2, 3, 4, 5, 17, 25];
    
    for (int i = 0; i < 6; i = i + 1) {
        int num = test_numbers[i];
        bool prime = is_prime(num);
        if (prime) {
            println(num, " is prime");
        } else {
            println(num, " is not prime");
        }
    }
    
    println("\nFinding all primes up to 20:");
    int prime_count = count_primes(20);
    println("Total primes found: ", prime_count);
    
    return 0;
}
