#include <std>
int main() {
    // Test simple list get method
    list<int> numbers = [10, 20, 30, 40];
    println("Numbers:");
    println(numbers);
    
    int first = numbers.get(0);
    int second = numbers.get(1);
    int third = numbers.get(2);
    int fourth = numbers.get(3);
    
    println("First element:");
    print(first);
    println("");
    println("Second element:");
    print(second);
    println("");
    println("Third element:");
    print(third);
    println("");
    println("Fourth element:");
    print(fourth);
    println("");
    
    // Test nested list get method
    list<list<int>> matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]];
    println("Matrix:");
    println(matrix);
    
    list<int> row1 = matrix.get(0);
    list<int> row2 = matrix.get(1);
    list<int> row3 = matrix.get(2);
    
    println("First row:");
    println(row1);
    println("Second row:");
    println(row2);
    println("Third row:");
    println(row3);
    
    // Test chained get calls
    int element_00 = row1.get(0);
    int element_01 = row1.get(1);
    int element_02 = row1.get(2);
    
    println("First row elements:");
    print(element_00);
    print(" ");
    print(element_01);
    print(" ");
    print(element_02);
    println("");
    
    return 0;
}
