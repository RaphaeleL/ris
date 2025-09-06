int main() {
    int arr[3];
    
    // Initialize array
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    
    // Access valid elements
    println(arr[0]);
    println(arr[1]);
    println(arr[2]);
    
    // This should cause bounds check failure and exit
    int invalid = arr[5];
    println(invalid);
    
    return 0;
}
