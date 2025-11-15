#include <stdlib.h>
#define SHL_IMPLEMENTATION
#define SHL_STRIP_PREFIX
#include "./build.h"

Cmd cmd = {0};
Timer timer = {0};

int main() {
    auto_rebuild("build.c");
    timer_start(&timer);

    char *source_files[][2] = {
        {"src/ast.cpp", "out/build/ast.o"},
        {"src/codegen.cpp", "out/build/codegen.o"},
        {"src/diagnostics.cpp", "out/build/diagnostics.o"},
        {"src/lexer.cpp", "out/build/lexer.o"},
        {"src/main.cpp", "out/build/main.o"},
        {"src/parser.cpp", "out/build/parser.o"},
        {"src/semantic_analyzer.cpp", "out/build/semantic_analyzer.o"},
        {"src/std.cpp", "out/build/std.o"},
        {"src/symbol_table.cpp", "out/build/symbol_table.o"},
        {"src/token.cpp", "out/build/token.o"},
        {"src/types.cpp", "out/build/types.o"},
    };

    mkdir_if_not_exists("out");
    mkdir_if_not_exists("out/build");
    mkdir_if_not_exists("out/bin");

    for (size_t i = 0; i < ARRAY_LEN(source_files); i++) {
        push(&cmd, "clang++");
        push(&cmd, "-c");
        push(&cmd, source_files[i][0]);
        push(&cmd, "-std=c++17", "-Wall", "-Wextra", "-O2", "-g",
             "-Wno-unused-parameter", "-Wno-deprecated-declarations", "-std=c++17",
             "-stdlib=libc++", "-fno-exceptions", "-funwind-tables",
             "-DEXPERIMENTAL_KEY_INSTRUCTIONS", "-D__STDC_CONSTANT_MACROS",
             "-D__STDC_FORMAT_MACROS", "-D__STDC_LIMIT_MACROS");
        push(&cmd, "-I/opt/homebrew/opt/llvm/include", "-Iinclude");
        push(&cmd, "-o", source_files[i][1]);
        if (!run(&cmd)) return EXIT_FAILURE;
    }

    push(&cmd, "ar", "rcs", "runtime/std.a", "out/build/std.o");
    if (!run_always(&cmd)) return EXIT_FAILURE;

    push(&cmd, "clang++", "-std=c++17", "-Wall", "-Wextra", "-O2", "-g",
         "-Wno-unused-parameter", "-Wno-deprecated-declarations",
         "-stdlib=libc++", "-fno-exceptions", "-funwind-tables",
         "-DEXPERIMENTAL_KEY_INSTRUCTIONS", "-D__STDC_CONSTANT_MACROS",
         "-D__STDC_FORMAT_MACROS", "-D__STDC_LIMIT_MACROS", "--sysroot",
         "$(xcrun --show-sdk-path)", "-L/opt/homebrew/opt/llvm/lib",
         "out/build/ast.o", "out/build/codegen.o", "out/build/diagnostics.o",
         "out/build/lexer.o", "out/build/main.o", "out/build/parser.o",
         "out/build/semantic_analyzer.o", "out/build/std.o",
         "out/build/symbol_table.o", "out/build/token.o", "out/build/types.o",
         "runtime/std.a", "-lLLVM", "-o", "out/bin/risc");

    if (!run(&cmd)) return EXIT_FAILURE;

    double elapsed_ms = timer_elapsed(&timer);
    timer_reset(&timer);
    info("Finished in %.3f seconds.\n", elapsed_ms);

    return EXIT_SUCCESS;
}
