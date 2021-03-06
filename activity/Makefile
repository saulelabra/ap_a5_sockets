# Simple example of a Makefile

### Variables for this project ###
# These should be the only ones that need to be modified
# The files that must be compiled, with a .o extension
OBJECTS_SERVER = server.o sockets.o
OBJECTS_CLIENT = client.o sockets.o
# The header files
DEPENDS = sockets.h
# The executable program to be created
SERVER = server
CLIENT = client

### Variables for the compilation rules ###
# These should work for most projects, but can be modified when necessary
# The compiler program to use
CC = gcc
# Options to use when compiling object files
CFLAGS = -Wall -g -std=gnu99 -pedantic
# Options to use for the final linking process
# This one links the math library
LDFLAGS = -lm

### The rules ###
# These should work for most projects without change
# Special variable meanings:
#   $@  = The name of the rule
#   $^  = All the requirements for the rule
#   $<  = The first required file of the rule

# Default rule
all: $(SERVER) $(CLIENT)

# Rule to make the executable
$(SERVER): $(OBJECTS_SERVER)
	$(CC) $^ -o $@ $(LDFLAGS)

# Rule to make the executable
$(CLIENT): $(OBJECTS_CLIENT)
	$(CC) $^ -o $@ $(LDFLAGS)

# Rule to make the object files
%.o: %.c $(DEPENDS)
	$(CC) $< -c -o $@ $(CFLAGS)

# Clear the compiled files
clean:
	rm -rf *.o $(MAIN) $(MAIN).zip
	
# Create a zip with the source code of the project
# Useful for submitting assignments
# Will clean the compilation first
zip: clean
	zip -r $(MAIN).zip *
	
# Indicate the rules that do not refer to a file
.PHONY: clean all zip
