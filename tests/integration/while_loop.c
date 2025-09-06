int main() {
    int x = 50;
    int count = 0;
    
    while (x > 0) {
        x = x - 1;
        count = count + 1;
    }
    
    println(count);
    return count;
}
