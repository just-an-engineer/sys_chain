#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "bitmap.h"
#include "io.h"

#ifdef USERSPACE
    #define DO_SYSCALL _DO_SYSCALL_USERSPACE
#else
    #define DO_SYSCALL _DO_SYSCALL_KERNEL
#endif

#define SET_REGISTER(n, register) \
    if ((bitmap->sym_args >> (n)) & 1) { \
        if (*(*buf_ptr) >= cur_syscall) { return -1; } \
        value = buffer[*(*buf_ptr)]; \
        printd("Symbolic value detected, setting register %d to 0x%lx, result of %ld\n", n, value, *(*buf_ptr)); \
    } else { \
        value = *(*buf_ptr); \
    } \
    register = value; \
    bitmap->num_args--; \
    (*buf_ptr)--;

typedef struct regs {
    unsigned long r0;
    unsigned long r1;
    unsigned long r2;
    unsigned long r3;
    unsigned long r4;
    unsigned long r5;
} regs;

#define reg_t unsigned long
long populate_arguments(unsigned long **buf_ptr, unsigned long *buffer, unsigned long cur_syscall, expanded_bitmap *bitmap, regs *regs) {
    unsigned long value;
    const unsigned long num_args_copy = bitmap->num_args;
    *buf_ptr += bitmap->num_args-1;
    switch (bitmap->num_args) {
        case 6:
            SET_REGISTER(5, regs->r5);
        case 5:
            SET_REGISTER(4, regs->r4);
        case 4:
            SET_REGISTER(3, regs->r3);
        case 3:
            SET_REGISTER(2, regs->r2);
        case 2:
            SET_REGISTER(1, regs->r1);
        case 1:
            SET_REGISTER(0, regs->r0);
        case 0:
            break;
        default:
            // illegal number of arguments
            return -1;
    }
    *buf_ptr += num_args_copy+1;
    return 0;
}

#endif // _SYSCALL_H