// Operator test file
int main() {
    int a = 10;
    int b = 5;
    
    // Arithmetic operators
    int sum = a + b;
    int diff = a - b;
    int prod = a * b;
    int quot = a / b;
    int rem = a % b;
    
    // Comparison operators
    bool eq = (a == b);
    bool ne = (a != b);
    bool lt = (a < b);
    bool gt = (a > b);
    bool le = (a <= b);
    bool ge = (a >= b);
    
    // Logical operators
    bool and_result = (a > 0) && (b > 0);
    bool or_result = (a < 0) || (b < 0);
    bool not_result = !(a == 0);
    
    return 0;
}
