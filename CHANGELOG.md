# Changelog

## [T050] - 2024-12-19

### Added
- Complete runtime library implementation with basic I/O functions
- Runtime function declarations for all basic types (int, float, bool, char, string)
- Print and println functions for all primitive types
- Memory management functions (malloc, free)
- String operations (concat, length)
- Array operations (alloc, free)
- Program exit function
- Runtime library integration with compiler and linker
- Semantic analyzer integration with runtime function declarations
- Type system compatibility between language and runtime (bool->i8 mapping)

### Technical Details
- C-style runtime library with extern "C" declarations
- Runtime functions: ris_print_*, ris_println_*, ris_malloc, ris_free, ris_string_*, ris_array_*, ris_exit
- Runtime library compiled to static archive (libris_runtime.a)
- Semantic analyzer populates symbol table with runtime function declarations
- Code generator declares external LLVM functions for runtime
- Type mapping: bool maps to i8 (int8_t) for runtime compatibility
- Integration test demonstrates full compilation pipeline

### Acceptance Criteria Met
- ✅ Runtime library provides basic I/O for all primitive types
- ✅ Memory management functions implemented
- ✅ String and array operations available
- ✅ Integration test compiles and runs successfully
- ✅ All unit tests pass (42/42 test cases)
- ✅ Full compilation pipeline: source -> tokens -> AST -> semantic analysis -> LLVM IR -> executable

## [T040] - 2024-12-19

### Added
- Complete LLVM IR code generator implementation
- Support for basic function code generation (int, float, bool, char, string, void)
- Global variable code generation with initializers
- Expression code generation (literals, identifiers, binary operations, unary operations)
- Function call code generation
- Return statement code generation
- LLVM module verification and IR output to file
- Comprehensive code generator unit test suite with 9 test cases
- Integration with main compiler pipeline

### Technical Details
- LLVM IR backend using LLVM 21.1.0
- Type mapping from language types to LLVM types (int->i64, float->double, bool->i1, char->i8, string->i8*, void->void)
- Function generation with proper calling conventions
- Global variable generation with proper linkage and initialization
- Expression code generation with LLVM instruction builders
- Basic block management for function bodies
- Module verification to ensure valid LLVM IR
- Error handling and reporting for code generation failures

### Acceptance Criteria Met
- ✅ All code generator unit tests pass (9/9 test cases)
- ✅ Translation from AST to LLVM IR for expressions, functions, and simple types
- ✅ Generated LLVM IR compiles with clang and produces correct executable output
- ✅ Support for basic functions, global variables, and return statements
- ✅ Proper type mapping and code generation
- ✅ Integration tests: `int main() { return 42; }` compiles and returns exit code 42

## [T030] - 2024-12-19

### Added
- Complete semantic analyzer implementation with type checking
- Symbol table management with scoping support
- Type system with primitive types (int, float, bool, char, string, void)
- Function signature validation and parameter checking
- Variable declaration analysis with type inference
- Expression type analysis for all language constructs
- Comprehensive semantic analyzer unit test suite with 15 test cases
- Error reporting with source position information

### Technical Details
- Symbol table with hierarchical scoping (global, function, block scopes)
- Type system supporting primitive types, arrays, functions, and structs
- Expression type analysis for literals, identifiers, binary operations, unary operations
- Type compatibility checking for assignments and function calls
- Boolean and arithmetic operation validation
- Array bounds checking and type validation
- Function call argument count and type validation
- Implicit type conversion support (int to float)

### Acceptance Criteria Met
- ✅ All semantic analyzer unit tests pass (15/15 test cases)
- ✅ Type checking for all language constructs
- ✅ Scoping and symbol table management
- ✅ Function signature validation
- ✅ Error reporting with clear messages
- ✅ Boolean operations and unary operators working correctly

## [T020] - 2024-12-19

### Added
- Complete parser implementation with recursive-descent parsing
- AST node definitions for all language constructs
- Support for functions, variables, control flow, and expressions
- VOID type support for function return types
- Comprehensive parser unit test suite with 14 test cases
- Parser integration with main compiler

### Technical Details
- Recursive-descent parser with proper precedence handling
- AST nodes for Program, FuncDecl, VarDecl, BlockStmt, IfStmt, WhileStmt, ForStmt, ReturnStmt, ExprStmt
- Expression parsing with operator precedence (assignment, logical, equality, comparison, term, factor, unary)
- Function call and array access expression support
- Error handling with source position reporting
- Parser takes tokens directly instead of lexer reference

### Acceptance Criteria Met
- ✅ All parser unit tests pass (14/14 test cases)
- ✅ Parser correctly handles functions, variables, control flow
- ✅ Expression parsing with proper precedence
- ✅ Error handling and recovery
- ✅ AST generation for all language constructs

## [T010] - 2024-12-19

### Added
- Complete lexer implementation with token definitions
- Token types for all language constructs (keywords, literals, operators, punctuation)
- Source position tracking for error reporting
- Comment handling (single-line // and multi-line /* */)
- String and character literal support with escape sequences
- Comprehensive unit test suite with 9 test cases
- Lexer test files for validation

### Technical Details
- Deterministic state machine tokenizer
- Handles all C-like language tokens without pointers
- Error handling without exceptions (LLVM compatibility)
- Proper EOF token handling
- Support for identifiers, keywords, literals, operators, punctuation
- Escape sequence support for strings and characters

### Acceptance Criteria Met
- ✅ All unit tests pass (9/9 test cases)
- ✅ Lexer correctly tokenizes all language tokens
- ✅ Comment handling works correctly
- ✅ Error handling works without exceptions
- ✅ Test files tokenized into expected sequences

## [T000] - 2024-12-19

### Added
- Project skeleton with C++ implementation
- Makefile build system with LLVM integration
- Basic directory structure (src/, include/, tests/)
- README.md with project documentation
- .gitignore for build artifacts
- Basic main.cpp with command-line interface
- Unit test framework and basic test
- Integration test structure
- LLVM version checking (requires LLVM >= 12)

### Technical Details
- Uses clang++ as C++ compiler
- LLVM 21.1.0 detected and working
- Build system supports unit tests and integration tests
- Basic command-line interface implemented
- Project structure follows single-main workflow

### Acceptance Criteria Met
- ✅ `make check` returns installed LLVM >= 12, prints tool versions
- ✅ Build system creates executable compiler
- ✅ Unit tests pass
- ✅ Basic command-line interface works
