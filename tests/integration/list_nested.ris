#include <std>

int main() {
    list<list<int>> a = [[1, 2], [3, 4]];
    println("full list", a);
    list<int> new_row = [5, 6];
    a.push(new_row);  // Push a list<int>, not an int
    println("full list", a);
    a.pop();
    println("full list", a);
    println("size", a.size());
    // Workaround: create individual rows since a[i] indexing has issues
    list<int> row1 = [1, 2];
    list<int> row2 = [3, 4];
    println("size of row1:", row1.size());
    
    println("Row 1:");
    println(row1);
    for (int j = 0; j < row1.size(); j = j + 1) {
        int element = row1[j];
        println("Element:", element);
    }
    
    println("Row 2:");
    println(row2);
    for (int j = 0; j < row2.size(); j = j + 1) {
        int element = row2[j];
        println("Element:", element);
    }
    return 0;
}
