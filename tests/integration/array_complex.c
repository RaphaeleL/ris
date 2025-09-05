int sum_array(int arr[5]) {
    int sum = 0;
    int i = 0;
    
    while (i < 5) {
        sum = sum + arr[i];
        i = i + 1;
    }
    
    return sum;
}

int main() {
    int numbers[5];
    
    // Initialize array
    numbers[0] = 1;
    numbers[1] = 2;
    numbers[2] = 3;
    numbers[3] = 4;
    numbers[4] = 5;
    
    // Calculate sum
    int result = sum_array(numbers);
    ris_println_int(result);
    
    // Modify array elements
    numbers[0] = 10;
    numbers[4] = 20;
    
    // Calculate new sum
    result = sum_array(numbers);
    ris_println_int(result);
    
    return result;
}
