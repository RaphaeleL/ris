#include <std>
int main() {
    int sum = 0;
    
    for (int i = 0; i < 5; i = i + 1) {
        sum = sum + i;
        if (i == 3) {
            println(i);
        }
        if (i == 2) {
            continue;
        }

    }
    
    println(sum);
    return sum;
}
