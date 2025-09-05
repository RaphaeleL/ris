int main() {
    bool arr[4];
    
    // Initialize array
    arr[0] = true;
    arr[1] = false;
    arr[2] = true;
    arr[3] = false;
    
    // Access and print elements
    ris_println_bool(arr[0]);
    ris_println_bool(arr[1]);
    ris_println_bool(arr[2]);
    ris_println_bool(arr[3]);
    
    return 0;
}
