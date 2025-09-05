# Compiler and flags
CC := gcc
CFLAGS := -std=c99 -Wall -Werror
LDFLAGS :=
BUILD_DIR := $(CURDIR)/build
TEST_INTPUT_DIR := $(CURDIR)/test_inputs
TEST_OUTPUT_DIR := $(CURDIR)/test_outputs

# Test inputs
TEST_FILE_NAMES := test0.asm test1.asm test2.asm mul_bytes.asm

# Path the inputs
ASM_FILES := $(addprefix $(TEST_INTPUT_DIR)/, $(TEST_FILE_NAMES))
HEX_FILES := $(addsuffix .hex, $(addprefix $(TEST_OUTPUT_DIR)/, $(basename $(TEST_FILE_NAMES))))
HEX_ANSWER_FILES := $(addsuffix _ref.hex, $(addprefix $(TEST_OUTPUT_DIR)/, $(basename $(TEST_FILE_NAMES))))

# Final assembler executable
TARGET := $(BUILD_DIR)/assembler.a

# Default target
all: $(TARGET)

# Build object file
$(BUILD_DIR)/assembler.o: assembler.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link to final executable
$(TARGET): $(BUILD_DIR)/assembler.o
	$(CC) $(LDFLAGS) $^ -o $@ 

# Run all assemblers
proc_all: proc_asm proc_ref

# Run our assembler for all test files
proc_asm: $(HEX_FILES)
	@echo "Assembled all test files with custom assembler."

# Run reference assembler for all test files
proc_ref: $(HEX_ANSWER_FILES)
	@echo "Processed all test files with reference assembler"

$(TEST_OUTPUT_DIR)/%_ref.hex: $(TEST_INTPUT_DIR)/%.asm $(TARGET) | $(TEST_OUTPUT_DIR)
	$(CURDIR)/assembler.linux $< $@

# Rule to turn .asm to .hex using the assembler
$(TEST_OUTPUT_DIR)/%.hex: $(TEST_INTPUT_DIR)/%.asm $(TARGET) | $(TEST_OUTPUT_DIR)
	@if [ -f "$@" ]; then rm "$@"; fi
	$(TARGET) $< $@

# Test output directory
$(TEST_OUTPUT_DIR):
	mkdir -p $@

# Make build directory
$(BUILD_DIR):
	mkdir -p $@

# Clean build and outputs
clean:
	-rm -rf $(BUILD_DIR)/* $(TEST_OUTPUT_DIR)/*
