# Highkey overkill for what we are doing here lmao

CC := gcc
CFLAGS := -std=c99 -Werror
LDFLAGS :=
BUILD_DIR := $(CURDIR)/build

DEFAULT_INPUT_FILE := $(CURDIR)/test_files/test2.asm
DEFAULT_OUTPUT_FILE := $(patsubst %.asm,%.hex,$(DEFAULT_INPUT_FILE))

TARGET := $(BUILD_DIR)/assembler.a

# Runs build target
all: $(TARGET)

# Removes existing hex file destination if it exists. The runs the target program with the default input/output file paths
proc_default: $(TARGET)
ifneq ("$(wildcard $(DEFAULT_OUTPUT_FILE))","")
	rm $(DEFAULT_OUTPUT_FILE)
endif
	$(TARGET) $(DEFAULT_INPUT_FILE) $(DEFAULT_OUTPUT_FILE)

# Links object files to the final executable
$(TARGET): $(BUILD_DIR)/assembler.o
	$(CC) $(LDFLAGS) $^ -o $@

# Creates the assmebler.o object file in the build directory
$(BUILD_DIR)/assembler.o: assembler.c | $(BUILD_DIR)
	$(CC) $(CFLGAS) -c $< -o $@

# Target to create the build directory if it does not exist
$(BUILD_DIR):
	mkdir $@

# Cleans the build directory
clean:
	-rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/*.a
