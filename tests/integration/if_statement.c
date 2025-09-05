int main() {
    int x = -1;
    int result = 0;
    
    if (x > 0) {
        result = 1;
    } else {
        result = 0;
    }
    
    ris_println_int(result);
    return result;
}
