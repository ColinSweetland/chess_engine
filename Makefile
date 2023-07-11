CC		:= clang++

# Warnings are good
CFLAGS	:= -Wall -Wextra -Werror -Wpedantic

# I probably shouldn't use the non-standard feature, but it's awesome
CFLAGS	+= -Wno-gnu-case-range

# C++ std
CFLAGS	+= -std=c++14

# use bmi instruction set (currently required but should not be)
CFLAGS	+= -mbmi2

# generate .d files which help make
CFLAGS	+= -MMD -MP

# start with empty postfix
EXE_POSTFIX :=

ifdef RELEASE 
# most optimization
CFLAGS	+= -O3
# link time optimization
CFLAGS	+= -flto
# native architechture
CFLAGS	+= -march=native

# add postfix to output
EXE_POSTFIX += -release

else # not optimized ('debug mode') 
# least optimization
CFLAGS	+= -O0
# most debug symbols
CFLAGS 	+= -g3
# sanitizers
CFLAGS	+= -fsanitize=undefined,address

EXE_POSTFIX += -debug
endif

SRC_DIR	:= src
OBJ_DIR	:= obj

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)

OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%$(EXE_POSTFIX).o,$(SOURCES))

OUT_DIR	 := output
EXE_NAME := chessengine
OUT_EXE	 := $(OUT_DIR)/$(EXE_NAME)$(EXE_POSTFIX)

DEPS = $(OBJECTS:.o=.d)

-include $(DEPS)

# Compile the program
all: $(OUT_EXE) 

# Run the program
run: $(OUT_EXE)
	echo "============= running program ============="; echo 
	./$(OUT_EXE); \
	echo "============= Exit code: $$? ================"
	

# link objects and output runnable program in output/
$(OUT_EXE): $(OBJECTS)
	mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Make a .o for each .cpp
$(OBJ_DIR)/%$(EXE_POSTFIX).o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Remove generated files
clean:
	rm -rf $(OBJ_DIR) $(OUT_DIR)

.PHONY: clean all run

.SILENT: run



