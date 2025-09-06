# RIS Compiler Makefile
# C++ implementation with LLVM backend

# Compiler and tools
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g -Wno-unused-parameter -Wno-deprecated-declarations
LLVM_CONFIG = llvm-config
LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags)
LLVM_LDFLAGS  = $(shell $(LLVM_CONFIG) --ldflags)
LLVM_LIBS     = $(shell $(LLVM_CONFIG) --libs core support)

# Directories
SRC_DIR     = src
INCLUDE_DIR = include
BUILD_DIR   = out/build
BIN_DIR     = out/bin
TEST_DIR    = tests
RUNTIME_DIR = runtime

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

# Test files
UNIT_TESTS      = $(wildcard $(TEST_DIR)/unit/*_test.cpp)
TEST_RUNNER     = $(TEST_DIR)/unit/test_runner.cpp

# Main targets
TARGET      = $(BIN_DIR)/risc
TEST_TARGET = $(BIN_DIR)/risc_test
RUNTIME_LIB = $(RUNTIME_DIR)/std.a

# Test object files
TEST_OBJECTS    = $(UNIT_TESTS:$(TEST_DIR)/unit/%_test.cpp=$(BUILD_DIR)/%_test.o)
TEST_RUNNER_OBJ = $(BUILD_DIR)/test_runner.o

# Pretty-print helpers
ECHO_CC = @printf " CXX     %s\n" $<
ECHO_LD = @printf " LD      %s\n" $@
ECHO_MK = @printf " MKDIR   %s\n" $@
ECHO_AR = @printf " AR      %s\n" $@
ECHO_RM = @printf " RM      %s\n" $<
ECHO_CP = @printf " CP      %s\n" $<

# Default target
all: $(TARGET)

# Create directories
$(BUILD_DIR):
	$(ECHO_MK) $@
	@mkdir -p $@

$(BIN_DIR):
	$(ECHO_MK) $@
	@mkdir -p $@

# Runtime library
$(RUNTIME_LIB): $(BUILD_DIR)/std.o | $(RUNTIME_DIR)
	$(ECHO_AR)
	@ar rcs $@ $^

# Main compiler executable
$(TARGET): $(OBJECTS) $(RUNTIME_LIB) | $(BIN_DIR)
	$(ECHO_LD)
	@$(CXX) $(CXXFLAGS) $(LLVM_LDFLAGS) -o $@ $^ $(LLVM_LIBS)

# Object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(ECHO_CC)
	@$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Test executable
$(TEST_TARGET): $(TEST_RUNNER_OBJ) $(TEST_OBJECTS) $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS)) | $(BIN_DIR)
	$(ECHO_LD)
	@$(CXX) $(CXXFLAGS) $(LLVM_LDFLAGS) -o $@ $^ $(LLVM_LIBS)

# Test object files
$(BUILD_DIR)/%_test.o: $(TEST_DIR)/unit/%_test.cpp $(HEADERS) | $(BUILD_DIR)
	$(ECHO_CC)
	@$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Test runner object file
$(TEST_RUNNER_OBJ): $(TEST_RUNNER) $(HEADERS) | $(BUILD_DIR)
	$(ECHO_CC)
	@$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Check LLVM installation
check:
	@echo "Checking LLVM installation..."
	@$(LLVM_CONFIG) --version
	@echo "LLVM CXX flags: $(LLVM_CXXFLAGS)"
	@echo "LLVM libraries: $(LLVM_LIBS)"
	@echo "LLVM version check:"
	@$(LLVM_CONFIG) --version | grep -E "([0-9]+)\.([0-9]+)" | \
		awk -F. '{if ($$1 >= 12) print "✓ LLVM version " $$0 " is >= 12"; else print "✗ LLVM version " $$0 " is < 12"; exit ($$1 < 12)}'

# Run unit tests
test: $(TEST_TARGET)
	@echo "Running unit tests..."
	@./$(TEST_TARGET)

# Clean build artifacts
clean:
	$(ECHO_RM) out
	@rm -rf out

# Install (optional)
install: $(TARGET)
	$(ECHO_CP) $(TARGET) /usr/local/bin/
	@cp $(TARGET) /usr/local/bin/

# Help
help:
	@echo "Available targets:"
	@echo "  all              - Build the compiler"
	@echo "  check            - Check LLVM installation"
	@echo "  test             - Run unit tests"
	@echo "  clean            - Clean build artifacts"
	@echo "  install          - Install compiler to /usr/local/bin"
	@echo "  help             - Show this help"

.PHONY: all check test clean install help