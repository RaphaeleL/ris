int main() {
    int arr[3];
    
    // Initialize array
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    
    // Access valid elements
    ris_println_int(arr[0]);
    ris_println_int(arr[1]);
    ris_println_int(arr[2]);
    
    // This should cause bounds check failure and exit
    int invalid = arr[5];
    ris_println_int(invalid);
    
    return 0;
}
