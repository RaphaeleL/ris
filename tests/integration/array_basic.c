int main() {
    int arr[5];
    
    // Initialize array
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    arr[3] = 40;
    arr[4] = 50;
    
    // Access and print elements
    ris_println_int(arr[0]);
    ris_println_int(arr[1]);
    ris_println_int(arr[2]);
    ris_println_int(arr[3]);
    ris_println_int(arr[4]);
    
    return arr[2];
}
