CC		:= clang
CFLAGS	:= -Wall -Wextra -Werror -Wpedantic -Wno-unused-variable -g -mbmi2 -std=c99 -O3 -MMD -MP

SRC_DIR	:= src
OBJ_DIR	:= obj

SOURCES := $(wildcard $(SRC_DIR)/*.c)

OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

OUT_DIR	:= output
OUTPUT 	:= $(OUT_DIR)/main

DEPS = $(OBJECTS:.o=.d)

-include $(DEPS)

# Compile than run the program
all: $(OUTPUT) run

# Run the program
run:
	./$(OUTPUT)

# link objects and output runnable program in output/
$(OUTPUT): $(OBJECTS)
	mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Make a .o for each .c
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Remove generated files
clean:
	rm -rf $(OBJ_DIR) $(OUT_DIR)

.PHONY: clean all run



