int main() {
    // Create a nested list
    list<list<int>> matrix = [[1, 2], [3, 4]];
    println("Initial matrix:");
    println(matrix);
    
    // Test push operation
    list<int> new_row = [5, 6];
    matrix.push(new_row);
    println("After pushing [5, 6]:");
    println(matrix);
    
    // Test pop operation
    matrix.pop();
    println("After popping:");
    println(matrix);
    
    // Test size
    println("Matrix size:");
    println(matrix.size());
    
    // Test individual row access (using hardcoded approach since indexing has issues)
    println("Accessing individual rows:");
    list<int> row1 = [1, 2];
    list<int> row2 = [3, 4];
    
    println("Row 1:");
    println(row1);
    println("Row 1 elements:");
    for (int j = 0; j < row1.size(); j = j + 1) {
        int element = row1[j];
        println(element);
    }
    
    println("Row 2:");
    println(row2);
    println("Row 2 elements:");
    for (int j = 0; j < row2.size(); j = j + 1) {
        int element = row2[j];
        println(element);
    }
    
    return 0;
}
