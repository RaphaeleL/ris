int sum(list<int> a) {
    int sum = 0;
    for (int i = 0; i < a.size(); i++) {
        sum = sum + a[i];
    }
    return sum;
}



int main() {
    list<int> a = [1, 2, 3, 4];
    println(a);
    a.push(5);
    println(a);
    a.pop();
    println(a);
    println(a.size());
    println(sum(a));
    return 0;
}
