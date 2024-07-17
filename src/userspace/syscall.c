// struct layout:
// unsigned long syscall
// unsigned long bitmap // [symbolic arguments][num of conditionals][number of arguments]
    // in this case we just use a char for each arg, and leave the remaining 5 bytes on 64 bit systems for future use
    // in reality, this can be optimized for each segment
    // the bitmap will likely be architecture size dependent
// unsigned long args
// ...
// error function address
// conditional enums, 2 per byte, no spilling between bytes. There is 1 unsigned long for every 8 conditionals
// conditional values to compare against

#include "syscall.h"
#include "../lib/asm.h"
#include "../lib/lib.h"
#include "../lib/syscall.h"
#include <string.h>

unsigned long syscall_chain(unsigned long size, unsigned long* address, unsigned long* result_buffer) {
    unsigned long *buffer = (unsigned long *)malloc(sizeof(unsigned long) * size);
    // pointer checking
    memcpy(buffer, address, sizeof(unsigned long) * size);

    unsigned long *buf_ptr = buffer;
    unsigned long cur_syscall = 0;
    while (buf_ptr < buffer + size) {
        unsigned long syscall = get_next_val(buf_ptr);
        if (syscall == SYS_CHAIN_NUM || cur_syscall >= SYMBOLIC_SIZE) {
            // we can't run our own syscall, so currently, just return. We _could_ skip
            goto EXIT;
        }
        unsigned long bitmap = get_next_val(buf_ptr);
        expanded_bitmap bitmap_vals;
        populate_bitmap_values(&bitmap_vals, bitmap);
        printd("Setting up syscall %ld with %ld arg(s), %ld conditional(s), and a symbolic bitmap of 0x%lx, from a bitmap value of 0x%lx\n", syscall, bitmap_vals.num_args, bitmap_vals.num_cond, bitmap_vals.sym_args, bitmap);

        regs regs;
        regs.r0 = regs.r1 = regs.r2 = regs.r3 = regs.r4 = regs.r5 = 0;
        if (populate_arguments(&buf_ptr, buffer, cur_syscall, &bitmap_vals, &regs) < 0) {
            printd("Illegal number of arguments\n");
            goto EXIT;
        }

        if (buf_ptr > buffer + size) { // check if we've gone over the buffer
            printd("Buffer overflow\n");
            goto EXIT;
        }
        printd("Running syscall %ld with args:\n\tr0: 0x%lx\n\tr1: 0x%lx\n\tr2: 0x%lx\n\tr3: 0x%lx\n\tr4: 0x%lx\n\tr5: 0x%lx\n", syscall, regs.r0, regs.r1, regs.r2, regs.r3, regs.r4, regs.r5);
        unsigned long result;
        // this is where we would jump to the syscall function in the syscall table
        DO_SYSCALL(result, syscall, regs.r0, regs.r1, regs.r2, regs.r3, regs.r4, regs.r5);
        buffer[cur_syscall] = result;
        printd("Result: 0x%lx, saved to index %ld\n", result, cur_syscall);
        cur_syscall++;
        
        // TODO: Have each conditional have its own error function?
        // I want to use something other than an error function anyways
        void (*error_func)(unsigned long) = (void (*)(unsigned long int))get_next_val(buf_ptr);

        unsigned char cond[bitmap_vals.num_cond];
        populate_conditionals(bitmap_vals, &buf_ptr, cond);
        #define RET_TO_USER_ERROR   memcpy(result_buffer, buffer, sizeof(unsigned long) * cur_syscall); \
                                    free(buffer); \
                                    if (error_func == 0x0) {goto EXIT;} \
                                    error_func(result); 
        // Future TODOs: be able to use symbolic cond_vals to use previous syscall results in the current comparison
        for (long unsigned int j = 0; j < bitmap_vals.num_cond; j++) {
            unsigned long cond_val = get_next_val(buf_ptr);
            printd("Testing if %ld (when compared to %ld) is ", result, cond_val);
            switch (check_conditional(result, cond_val, cond[j])) {
                case 1:
                    // ret to user error
                    printd("True\n");
                    RET_TO_USER_ERROR
                    break;
                case 0:
                    // continue
                    printd("False\n");
                    break;
                case -1:
                    // illegal conditional
                    goto EXIT;
                default:
                    // should never happen
                    goto EXIT;
            }
        }
        printd("\n");
    }
    EXIT:
    // probably need to do address checking again
    memcpy(result_buffer, buffer, sizeof(unsigned long) * (cur_syscall+1));
    free(buffer);
    return cur_syscall-1; // return the last successful syscall, or -1 on an initial error 
}
