int main() {
    int sum = 0;
    
    for (int i = 0; i < 5; i = i + 1) {
        sum = sum + i;
        if (i == 3) {
            ris_println_int(i);
        }
    }
    
    ris_println_int(sum);
    return sum;
}
