int main() {
    int numbers[5];
    
    // Initialize array
    numbers[0] = 1;
    numbers[1] = 2;
    numbers[2] = 3;
    numbers[3] = 4;
    numbers[4] = 5;
    
    // Calculate sum
    int sum = 0;
    int i = 0;
    
    while (i < 5) {
        sum = sum + numbers[i];
        i = i + 1;
    }
    
    ris_println_int(sum);
    
    // Modify array elements
    numbers[0] = 10;
    numbers[4] = 20;
    
    // Calculate new sum
    sum = 0;
    i = 0;
    
    while (i < 5) {
        sum = sum + numbers[i];
        i = i + 1;
    }
    
    ris_println_int(sum);
    
    return sum;
}
