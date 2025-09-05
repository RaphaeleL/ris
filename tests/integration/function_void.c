void print_hello() {
    ris_println_string("Hello, World!");
}

void print_numbers(int a, int b) {
    ris_println_int(a);
    ris_println_int(b);
}

int main() {
    print_hello();
    print_numbers(42, 84);
    return 0;
}
