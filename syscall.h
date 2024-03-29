#ifndef SYSCALL_H
#define SYSCALL_H
#include <string.h>

#include <stdio.h>

#define NUM_ARG_SIZE 3
#define CONDITION_SIZE 3
#define SYMBOLIC_SIZE 6

// you still need to define the bitmap of symbolic values
#define BITMAP_CREATOR(num_args, num_conditionals, symbolic_values) \
    (((unsigned long)num_args<<(SYMBOLIC_SIZE+CONDITION_SIZE)) | \
    ((unsigned long)num_conditionals << SYMBOLIC_SIZE) | \
    symbolic_values)

unsigned long syscall_chain(unsigned long size, unsigned long* address, unsigned long* result_buffer);

enum conditional_check {
    EQUAL,
    NOT_EQUAL,
    GREATER_SIGNED,
    GREATER_UNSIGNED,
    LESS_SIGNED,
    LESS_UNSIGNED,
    GREATER_EQUAL_SIGNED,
    GREATER_EQUAL_UNSIGNED,
    LESS_EQUAL_SIGNED,
    LESS_EQUAL_UNSIGNED,
};

#endif