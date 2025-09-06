#include <std>
int main() {
    // Test basic nested list creation and printing
    list<list<int>> matrix = [[1, 2], [3, 4], [5, 6]];
    println("Matrix:");
    println(matrix);
    
    // Test nested list with different inner list sizes
    list<list<string>> words = [["hello", "world"], ["test"], ["a", "b", "c"]];
    println("Words:");
    println(words);
    
    // Test nested list with mixed types
    list<list<bool>> flags = [[true, false], [false, true, false]];
    println("Flags:");
    println(flags);
    
    // Test nested list with single element inner lists
    list<list<int>> single = [[1], [2], [3]];
    println("Single element nested list:");
    println(single);
    
    return 0;
}
