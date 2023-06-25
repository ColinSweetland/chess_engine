CC		:= clang++

# Warnings & sanitizers are good
CFLAGS	:= -Wall -Wextra -Werror -Wpedantic
CFLAGS	+= -fsanitize=undefined,address

# I probably shouldn't use the non-standard feature, but it's awesome
CFLAGS	+= -Wno-gnu-case-range

# -g add debugging info -O optimization level
CFLAGS 	+= -g3
CFLAGS	+= -O0

# use bmi instruction set
CFLAGS	+= -mbmi2

# C++ std
CFLAGS	+= -std=c++14

# generate .d files which help make
CFLAGS	+= -MMD -MP

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
	echo "============= running program ============="; echo 
	./$(OUTPUT); \
	echo "============= Exit code: $$? ================"
	

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



