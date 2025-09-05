# RIS Compiler

A C-like compiler (without pointers) that compiles to LLVM IR and generates native executables.

## Architecture

This compiler implements a subset of C with the following features:
- **Types**: `int` (64-bit), `float` (64-bit double), `bool` (1-byte), `char` (8-bit), fixed-length arrays, `string` (immutable, heap-backed)
- **No pointers**: No pointer arithmetic, no address-of `&`, no dereference `*` operator
- **Control flow**: `if`, `else`, `while`, `for`, `break`, `continue`, `return`
- **Functions**: Typed parameters, single return type (void allowed)
- **Structs**: Value semantics, no unions
- **Operators**: Arithmetic, comparison, logical operators

## Building

### Prerequisites

- LLVM 12 or later
- Clang++ compiler
- Make

### Build Commands

```bash
# Check LLVM installation
make check

# Build the compiler
make

# Run unit tests
make test

# Run integration tests
make test-integration

# Clean build artifacts
make clean
```

## Usage

```bash
# Compile a C-like source file to LLVM IR
./bin/risc input.c -o output.ll

# Compile and run
./bin/risc input.c -o output.ll
llc output.ll -o output.s
clang output.s -o output
./output
```

## Project Structure

```
ris/
├── src/                 # C++ source files
├── include/            # Header files
├── tests/
│   ├── unit/          # Unit tests
│   ├── integration/   # Integration tests
│   └── lexer_tests/   # Lexer test files
├── build/             # Build artifacts
├── bin/               # Executables
├── Makefile          # Build system
└── README.md         # This file
```

## Development

This project follows a single-main workflow:
- All development happens on the `main` branch
- Each task is atomic and self-contained
- Tests must pass before any commit
- Cross-platform builds are verified before pushing

See `tasks.mdc` for the complete task list and `workflow.mdc` for development guidelines.
