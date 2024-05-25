# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c99

# Target executable
BINS = serial_linux

# Source files
SRCS = serial_linux.c
HDRS = serial_linux.h

# Object files
OBJS = $(SRCS:.c=.o)

# Main target
$(BINS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)
	rm -f $(OBJS)

# Compile source files
%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $<

# Clean objects and executable
clean:
	rm -f $(OBJS) $(BINS)