int multiply(int a, int b, int c) {
    return a * b * c;
}

float calculate(float x, float y, float z) {
    return x + y - z;
}

int main() {
    int result1 = multiply(2, 3, 4);
    ris_println_int(result1);
    
    float result2 = calculate(10.5, 5.2, 2.1);
    ris_println_float(result2);
    
    return result1;
}
