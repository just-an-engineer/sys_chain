# Compiler and flags
CC ?= clang
CFLAGS ?= -Wall -Wextra -Werror -g -Wno-implicit-fallthrough
DEFINES = -DDEBUG

SRC_DIRS = src/lib src/userspace

# Output executable name
EXEC = test

# Find all source files in the specified directories
SRC_FILES = $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))

INCLUDES = -Isrc/userspace -Iinclude

all: $(EXEC)

$(EXEC): $(SRC_FILES)
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) $^ -o $@

clean:
	rm -f $(EXEC)

.PHONY: all clean
