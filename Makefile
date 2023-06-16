CC		:= clang++
CFLAGS	:= -Wall -Wextra -Werror -Wpedantic -Wno-unused-variable -Wno-gnu-case-range -g -mbmi2 -std=c++17 -O1 -MMD -MP

SRC_DIR	:= src
OBJ_DIR	:= obj

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)

OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

OUT_DIR	:= output
OUTPUT 	:= $(OUT_DIR)/main

DEPS = $(OBJECTS:.o=.d)

-include $(DEPS)

# Compile than run the program
all: $(OUTPUT) run

# Run the program
run:
	echo "============= running program =============" && echo 
	./$(OUTPUT) && \
	echo "============= Exit code: $$? ==============="
	

# link objects and output runnable program in output/
$(OUTPUT): $(OBJECTS)
	mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Make a .o for each .cpp
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Remove generated files
clean:
	rm -rf $(OBJ_DIR) $(OUT_DIR)

.PHONY: clean all run

.SILENT: run



