# RIS Compiler Makefile
# C++ implementation with LLVM backend

# Compiler and tools
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
LLVM_CONFIG = llvm-config
LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags)
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags)
LLVM_LIBS = $(shell $(LLVM_CONFIG) --libs core support)

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
TEST_DIR = tests

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

# Test files
UNIT_TESTS = $(wildcard $(TEST_DIR)/unit/*_test.cpp)
TEST_RUNNER = $(TEST_DIR)/unit/test_runner.cpp
INTEGRATION_TESTS = $(wildcard $(TEST_DIR)/integration/*.c)

# Main targets
TARGET = $(BIN_DIR)/risc
TEST_TARGET = $(BIN_DIR)/risc_test

# Test object files
TEST_OBJECTS = $(UNIT_TESTS:$(TEST_DIR)/unit/%_test.cpp=$(BUILD_DIR)/%_test.o)
TEST_RUNNER_OBJ = $(BUILD_DIR)/test_runner.o

# Default target
all: $(TARGET)

# Create directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Main compiler executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LLVM_LDFLAGS) -o $@ $^ $(LLVM_LIBS) -lc++

# Object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Test executable
$(TEST_TARGET): $(TEST_RUNNER_OBJ) $(TEST_OBJECTS) $(filter-out build/main.o, $(OBJECTS)) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LLVM_LDFLAGS) -o $@ $^ $(LLVM_LIBS) -lc++

# Test object files
$(BUILD_DIR)/%_test.o: $(TEST_DIR)/unit/%_test.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Test runner object file
$(TEST_RUNNER_OBJ): $(TEST_RUNNER) $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

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

# Run integration tests
test-integration: $(TARGET)
	@echo "Running integration tests..."
	@for test in $(INTEGRATION_TESTS); do \
		echo "Testing $$test..."; \
		./$(TARGET) $$test -o /tmp/test_$$(basename $$test .c).ll; \
		llc /tmp/test_$$(basename $$test .c).ll -o /tmp/test_$$(basename $$test .c).s; \
		clang /tmp/test_$$(basename $$test .c).s -o /tmp/test_$$(basename $$test .c); \
		/tmp/test_$$(basename $$test .c); \
	done

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Install (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build the compiler"
	@echo "  check        - Check LLVM installation"
	@echo "  test         - Run unit tests"
	@echo "  test-integration - Run integration tests"
	@echo "  clean        - Clean build artifacts"
	@echo "  install      - Install compiler to /usr/local/bin"
	@echo "  help         - Show this help"

.PHONY: all check test test-integration clean install help
