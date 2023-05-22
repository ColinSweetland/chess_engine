CC		:= gcc
CFLAGS	:= -Wall -Wextra -Werror -Wpedantic -g -lm -std=c99 -O3 -MMD -MP

SRC		:= src
OBJ 	:= obj

SOURCES := $(wildcard $(SRC)/*.c)

OBJECTS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SOURCES))
OUTPUT	:= ./output/main

DEPS = $(OBJECTS:.o=.d)

-include $(DEPS)

# Compile than run the program
all: $(OUTPUT) run

# Run the program
run:
	$(OUTPUT)

# link objects and output runnable program in output/
$(OUTPUT): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

# Make a .o for each .c
$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Remove generated files
clean:
	rm -f $(OBJECTS) $(DEPS) $(OUTPUT)

.PHONY: clean all run



