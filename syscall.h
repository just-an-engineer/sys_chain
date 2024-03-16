#ifndef SYSCALL_H
#define SYSCALL_H
#include <string.h>

#include <stdio.h>

long syscall_negative_one(long size, long* address);

enum conditional_check {
    EQUAL,
    NOT_EQUAL,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL
};

#endif