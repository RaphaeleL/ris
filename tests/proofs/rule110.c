// Rule 110 Cellular Automaton Implementation
// Rule 110 is a 1D cellular automaton that has been proven to be Turing complete

#include <std>

int main() {
    println("Rule 110 Cellular Automaton");
    println("--------------------------");
    
    // Display the rule pattern first
    println("Rule 110 Pattern:");
    println("111 -> 0    011 -> 1");
    println("110 -> 1    010 -> 1");
    println("101 -> 1    001 -> 1");
    println("100 -> 0    000 -> 0");
    println();
    
    // Size of the cellular automaton
    int size = 50;
    
    // Number of generations to simulate
    int generations = size;
    
    // Initialize the cells - single cell at the end
    list<int> cells = [];
    for (int i = 0; i < size; i++) {
        if (i == size - 1) {
            cells.push(1);  // Single live cell at the end
        } else {
            cells.push(0);
        }
    }
    
    // Print the cells
    for (int i = 0; i < cells.size(); i = i + 1) {
        if (cells[i] == 1) {
            print("x ");  // Use a filled box for live cells
        } else {
            print("  ");  // Use an empty box for dead cells
        }
    }
    
    // Run the simulation for specified number of generations
    for (int gen = 1; gen <= generations; gen = gen + 1) {
        // Create a new list for the next generation
        list<int> next_generation = [];
        
        // Apply Rule 110 to every cell
        for (int i = 0; i < size; i = i + 1) {
            // Get the states of the current cell and its neighbors
            int left = 0;
            if (i > 0) {
                left = cells[i - 1];
            }
            
            int center = cells[i];
            
            int right = 0;
            if (i < size - 1) {
                right = cells[i + 1];
            }
            
            // Calculate the pattern (binary to decimal)
            int pattern = (left * 4) + (center * 2) + right;
            
            // Apply Rule 110:
            // pattern 7 (111) -> 0
            // pattern 6 (110) -> 1
            // pattern 5 (101) -> 1
            // pattern 4 (100) -> 0
            // pattern 3 (011) -> 1
            // pattern 2 (010) -> 1
            // pattern 1 (001) -> 1
            // pattern 0 (000) -> 0
            
            int next_state = 0;
            if (pattern == 1) {
                next_state = 1;
            }
            if (pattern == 2) {
                next_state = 1;
            }
            if (pattern == 3) {
                next_state = 1;
            }
            if (pattern == 5) {
                next_state = 1;
            }
            if (pattern == 6) {
                next_state = 1;
            }
            
            next_generation.push(next_state);
        }
        
        // Display the current generation
        println();
        // Print the next generation
        for (int i = 0; i < next_generation.size(); i = i + 1) {
            if (next_generation[i] == 1) {
                print("x ");  // Use a filled box for live cells
            } else {
                print("  ");  // Use an empty box for dead cells
            }
        }
        
        // Update cells for the next generation
        cells = next_generation;
    }
    
    // Final explanation
    println();
    println("Rule 110 is proven to be Turing complete,");
    println("meaning it can simulate any Turing machine and");
    println("therefore compute anything that is computable.");

    return 0;

}