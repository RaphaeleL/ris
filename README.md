## RIS — A tiny language with an LLVM backend

RIS is a small programming language and compiler implemented in C++17 that targets LLVM. It includes a tokenizer, parser, semantic analyzer, and LLVM IR code generator, plus a minimal runtime library (`runtime/std.a`).

### Features

- Lexer, parser, semantic analysis, codegen: end-to-end pipeline
- LLVM backend: emits IR and produces native executables via `llc` + `clang++`
- Standard library (opt-in): include with `#include <std>` to use `print`/`println`, basic types, etc.
- Cross-platform output: builds on macOS/Linux (Windows may require adjustments)

## Requirements

- C++17 compiler (Clang recommended)
- LLVM (12+ recommended)
  - Tools used: `llvm-config`, `llc`, `clang++`

## Build

```bash
make # builds the compiler at src → out/bin/risc
```

Artifacts:
- Compiler: `out/bin/risc`
- Build objects: `out/build/*.o`
- Runtime library: `runtime/std.a`

## Usage

Basic syntax:

```bash
out/bin/risc <input.ris> [-o <output>] [--run] [--verbose]
```

- -o <output>: output file name. If it does not end with `.ll`, an executable is produced; if it ends with `.ll`, LLVM IR is written instead.
- --run: run the produced executable after a successful build.
- --verbose: print compilation steps and details.

Notes:

- If no `-o` is omitted, the output name is derived from the input stem (e.g., `hello.ris` → `hello`).
- The standard library is linked automatically only if the source contains `#include <std>`.

## Examples

Examples can be found in `tests/integration/`.


## Tests

Unit tests:

```bash
make test
```

Integration examples are in `tests/integration/`. You can compile any of them, e.g.:

```bash
out/bin/risc tests/integration/function_basic.ris --run
```

## Project layout

- `include/` — public headers for the compiler components
- `src/` — compiler implementation and entrypoint
- `runtime/` — static runtime library (`std.a`)
- `tests/` — unit tests and integration samples
- `Makefile` — build, test, and utility targets

## Benchmark

> Each Language is tested with the same Rule110 implementation on the same Machine 

| Language                                      | Time (s) | Tested with | Note              |
|-----------------------------------------------|----------|-------------|-------------------|
| [ris](https://github.com/RaphaeleL/ris)       | 0.294s   | Rule110     | `--run`           |
| [ris](https://github.com/RaphaeleL/ris)       | 0.008s   | Rule110     | build and run     |
| [fulani](https://github.com/RaphaeleL/fulani) | 0.323s   | Rule110     |                   |
| Python                                        | 0.025s   | Rule110     | v3.13             |
| C                                             | 0.003s   | Rule110     | no `-O3`          |
| C++                                           | 0.205s   | Rule110     | no `-O3`          |
| Go                                            | 0.224s   | Rule110     |                   |
| Go                                            | 0.014s   | Rule110     | build and run     |


## Clean / Install

```bash
make clean
make -B
sudo make install
```

