int main() {
    int x = 10;
    int sum = 0;
    
    // Test if-else with while loop
    if (x > 5) {
        while (x > 0) {
            sum = sum + x;
            x = x - 1;
        }
    } else {
        sum = 0;
    }
    
    // Test for loop
    for (int i = 0; i < 3; i = i + 1) {
        sum = sum + i;
    }
    
    ris_println_int(sum);
    return sum;
}
