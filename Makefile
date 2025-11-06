CC = gcc
TARGET = app
# Compile flags
# -Wall = All warnings
# -Wextra = Extra warnings
# -g = Add debugging symbols
# -std=c11 = Use the C11 standard
CFLAGS = -Wall -Wextra -g -std=c11

LDFLAGS =

LIBS = -lpthread -lpaho-mqtt3c -lncurses

SRCS = $(shell find . -name "*.c")
OBJS = $(SRCS:.c=.o)

# -I. adds the root directory (for main.c to find other headers)
INCLUDES = -I. \
           -I./server \
           -I./mqtt \
           -I./config \
           -I./parser

# Add include directories to the compiler flags
CFLAGS += $(INCLUDES)

.PHONY: all
all: $(TARGET)

# Rule to link the final executable
$(TARGET): $(OBJS)
	@echo "LN $@"
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	@echo "Build complete: $(TARGET)"

%.o: %.c
	@echo "CC $<"
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET) $(OBJS)

# Rule to force a rebuild (clean + all)
.PHONY: re
re: clean all
