int main() {
    int x = 5;
    int count = 0;
    
    while (x > 0) {
        println(x);
        if (x == 3) {
            break;
        }
        if (x == 2) {
            continue;
        }
        x = x - 1;
        count = count + 1;
    }
    
    println(count);
    return count;
}
