int factorial(int n) {
    int result = 1;
    int i = 1;
    
    while (i <= n) {
        result = result * i;
        i = i + 1;
    }
    
    return result;
}

int main() {
    int result = factorial(5);
    ris_println_int(result);
    return result;
}
