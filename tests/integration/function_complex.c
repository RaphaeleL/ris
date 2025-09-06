#include <std>
int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int min(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

int calculate(int x, int y, int z) {
    int max_val = max(x, y);
    int min_val = min(max_val, z);
    return max_val - min_val;
}

int main() {
    int result = calculate(10, 5, 8);
    println(result);
    return result;
}
