int main() {
    list<int> a = [1, 2, 3];
    
    // Test push
    a.push(4);
    a.push(5);
    print(a);
    print(" ");
    
    // Test size
    int s = a.size();
    print(s);
    print(" ");
    
    // Test indexing
    int x = a[0];
    int y = a[2];
    print(x);
    print(" ");
    print(y);
    print(" ");
    
    // Test pop
    a.pop();
    a.pop();
    print(a);
    print(" ");
    
    // Test size after pop
    int s2 = a.size();
    print(s2);
    
    return 0;
}
