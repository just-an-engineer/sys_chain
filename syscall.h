#ifndef SYSCALL_H
#define SYSCALL_H
#include <string.h>

#include <stdio.h>

unsigned long syscall_negative_one(unsigned long size, unsigned long* address, unsigned long* result_buffer);

enum conditional_check {
    EQUAL,
    NOT_EQUAL,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL
};

#endif