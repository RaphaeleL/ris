# Changelog

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
