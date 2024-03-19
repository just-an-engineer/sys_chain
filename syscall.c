// struct layout:
// unsigned long syscall
// unsigned long bitmap // [symbolic arguments][num of conditionals][number of arguments]
    // in this case we just use a char for each arg, and leave the remaining 5 bytes on 64 bit systems for future use
    // in reality, this can be optimized for each segment
    // the bitmap will likely be architecture size dependent
// unsigned long args
// ...
    // the args need to be in reverse order, for ease of programming, currently
// error function address
// conditional enums, 2 per byte, no spilling between bytes. There is 1 unsigned long for every 8 conditionals
// conditional values to compare against

#include <stdlib.h>
#include "syscall.h"

#define SYS_CHAIN_NUM 500
#define NUM_ARG_SIZE_BITS ((1UL << NUM_ARG_SIZE) - 1)
#define CONDITION_SIZE_BITS ((1UL << CONDITION_SIZE) - 1)
#define SYMBOLIC_SIZE_BIT ((1UL << SYMBOLIC_SIZE) - 1)

#define SET_REGISTER(n, register) \
    if ((sym_args >> (n)) & 1) { \
        if (*(buf_ptr) >= cur_syscall) { goto EXIT; } \
        value = buffer[*(buf_ptr)]; \
        printf("Symbolic value detected, setting register %d to 0x%lx, result of %ld\n", n, value, *(buf_ptr)); \
    } else { \
        value = *(buf_ptr); \
    } \
    register = value; \
    num_args--; \
    buf_ptr--;

#ifdef __x86_64__
    #define SYSCALL asm("syscall");
    #define _64_BIT 1
#elif defined(__aarch64__)
    #define SYSCALL asm("svc 0");
    #define _64_BIT 1
#elif defined(__arm__)
    #define SYSCALL asm("swi 0");
    #define _64_BIT 0
#else
    #error "Unsupported architecture."
#endif

#if defined(__x86_64__)
    #define DO_SYSCALL(result_var, syscall_number, arg1, arg2, arg3, arg4, arg5, arg6) \
        __asm__ volatile ( \
            "mov %1, %%rax\n\t" \
            "mov %2, %%rdi\n\t" \
            "mov %3, %%rsi\n\t" \
            "mov %4, %%rdx\n\t" \
            "mov %5, %%r10\n\t" \
            "mov %6, %%r8\n\t" \
            "mov %7, %%r9\n\t" \
            "syscall\n\t" \
            "mov %%rax, %0" \
            : "=r"(result_var) \
            : "r"(syscall_number), "r"(arg1), "r"(arg2), \
              "r"(arg3), "r"(arg4), "r"(arg5), \
              "r"(arg6) \
            : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "cc", "memory")
#elif defined(__aarch64__)
    #define DO_SYSCALL(result_var, syscall_number, arg1, arg2, arg3, arg4, arg5, arg6) \
        __asm__ volatile ( \
            "mov x8, %1\n\t" \
            "mov x0, %2\n\t" \
            "mov x1, %3\n\t" \
            "mov x2, %4\n\t" \
            "mov x3, %5\n\t" \
            "mov x4, %6\n\t" \
            "mov x5, %7\n\t" \
            "svc #0\n\t" \
            "mov %0, x0" \
            : "=r"(result_var) \
            : "r"(syscall_number), "r"(arg1), "r"(arg2), \
              "r"(arg3), "r"(arg4), "r"(arg5), \
              "r"(arg6) \
            : "x0", "x1", "x2", "x3", "x4", "x5", "x8", "cc", "memory")
#elif defined(__arm__)
    #define DO_SYSCALL(result_var, syscall_number, arg1, arg2, arg3, arg4, arg5, arg6) \
        __asm__ volatile ( \
            "mov r7, %1\n\t" \
            "mov r0, %2\n\t" \
            "mov r1, %3\n\t" \
            "mov r2, %4\n\t" \
            "mov r3, %5\n\t" \
            "mov r4, %6\n\t" \
            "mov r5, %7\n\t" \
            "svc 0\n\t" \
            "mov %0, r0" \
            : "=r"(result_var) \
            : "r"(syscall_number), "r"(arg1), "r"(arg2), \
              "r"(arg3), "r"(arg4), "r"(arg5), \
              "r"(arg6) \
            : "r0", "r1", "r2", "r3", "r4", "r5", "r7", "cc", "memory")
#else
    #error "Unsupported architecture."
#endif


unsigned long syscall_chain(unsigned long size, unsigned long* address, unsigned long* result_buffer) {
    // TODO: verbose vs quiet mode. In production, likely remove all print statements, but for time being (v0), we can keep them
    unsigned long *buffer = (unsigned long *)malloc(sizeof(unsigned long) * size);
    // pointer checking
    memcpy(buffer, address, sizeof(unsigned long) * size);

    unsigned long *buf_ptr = buffer;
    unsigned long cur_syscall = 0;
    while (buf_ptr < buffer + size) {
        unsigned long syscall = *(buf_ptr);
        if (syscall == SYS_CHAIN_NUM || cur_syscall >= SYMBOLIC_SIZE) {
            // we can't run our own syscall, so currently, just return. We _could_ skip
            goto EXIT;
        }
        buf_ptr++;
        unsigned long bitmap = *(buf_ptr);
        buf_ptr++;
        unsigned long sym_args = bitmap & SYMBOLIC_SIZE_BIT;
        unsigned long num_cond = (bitmap >> SYMBOLIC_SIZE) & CONDITION_SIZE_BITS;
        unsigned long num_args = (bitmap >> (SYMBOLIC_SIZE+CONDITION_SIZE)) & NUM_ARG_SIZE_BITS;
        const unsigned long num_args_copy = num_args;

        printf("Setting up syscall %ld with %ld arg(s), %ld conditional(s), and a bitmap of 0x%lx\n", syscall, num_args, num_cond, bitmap);

        unsigned long r0, r1, r2, r3, r4, r5 = 0;
        
        unsigned long value;
        buf_ptr += num_args-1;
        switch (num_args) {
            case 6:
                SET_REGISTER(5, r5);
            case 5:
                SET_REGISTER(4, r4);
            case 4:
                SET_REGISTER(3, r3);
            case 3:
                SET_REGISTER(2, r2);
            case 2:
                SET_REGISTER(1, r1);
            case 1:
                SET_REGISTER(0, r0);
            case 0:
                break;
            default:
                // illegal number of arguments
                goto EXIT;
        }
        buf_ptr += num_args_copy+1;

        // this is where we would jump to the syscall function in the syscall table
        if (buf_ptr > buffer + size) { // check if we've gone over the buffer
            goto EXIT;
        }
        printf("Running syscall %ld with args:\n\tr0: 0x%lx\n\tr1: 0x%lx\n\tr2: 0x%lx\n\tr3: 0x%lx\n\tr4: 0x%lx\n\tr5: 0x%lx\n", syscall, r0, r1, r2, r3, r4, r5);
        unsigned long result;
        DO_SYSCALL(result, syscall, r0, r1, r2, r3, r4, r5);
        buffer[cur_syscall] = result;
        printf("Result: 0x%lx, saved to index %ld\n", result, cur_syscall);
        cur_syscall++;
        
        void (*error_func)(unsigned long) = (void (*)(unsigned long int))*(buf_ptr);
        buf_ptr++;

        unsigned char cond[num_cond];
        unsigned char mask = 0b1111;

        for (int j = 0; j < num_cond;) {
            #define GET_N_CONDITIONAL(n) \
                j<num_cond ? cond[j] = (cond_val >> (n * 4)) & mask : 0; \
                j++;
            unsigned long cond_val = *(buf_ptr);
            buf_ptr++;
            
            GET_N_CONDITIONAL(0);
            GET_N_CONDITIONAL(1);
            GET_N_CONDITIONAL(2);
            GET_N_CONDITIONAL(3);
            GET_N_CONDITIONAL(4);
            GET_N_CONDITIONAL(5);
            GET_N_CONDITIONAL(6);
            GET_N_CONDITIONAL(7);
            #if _64_BIT
                GET_N_CONDITIONAL(8);
                GET_N_CONDITIONAL(9);
                GET_N_CONDITIONAL(10);
                GET_N_CONDITIONAL(11);
                GET_N_CONDITIONAL(12);
                GET_N_CONDITIONAL(13);
                GET_N_CONDITIONAL(14);
                GET_N_CONDITIONAL(15);
            #endif
        }

        // Future TODOs: be able to use symbolic cond_vals to use previous syscall results in the current comparison
        for (int j = 0; j < num_cond; j++) {
            unsigned long cond_val = *(buf_ptr);     buf_ptr++;
            printf("Testing if %ld ( when compared to %ld) is ", result, cond_val);
            switch (cond[j]) {
                // this is where we would overwrite the rip or instruction pointer in the saved context, and return to userspace
                #define RET_TO_USER_ERROR   if (error_func == 0x0) {goto EXIT;} \
                                            free(buffer); \
                                            memcpy(result_buffer, buffer, sizeof(unsigned long) * cur_syscall); \
                                            error_func(result); 
                case EQUAL:
                    printf("equal\n");
                    if (result == cond_val) { RET_TO_USER_ERROR }
                    break;
                case NOT_EQUAL:
                    printf("not equal\n");
                    if (result != cond_val) { RET_TO_USER_ERROR }
                    break;
                case GREATER_SIGNED:
                    printf("greater than, signed\n");
                    if ((long)result > (long)cond_val) { RET_TO_USER_ERROR }
                    break;
                case LESS_SIGNED:
                    printf("less than, signed\n");
                    if ((long)result < (long)cond_val) { RET_TO_USER_ERROR }
                    break;
                case GREATER_EQUAL_SIGNED:
                    printf("greater than or equal to, signed\n");
                    if ((long)result >= (long)cond_val) { RET_TO_USER_ERROR }
                    break;
                case LESS_EQUAL_SIGNED:
                    printf("less than or equal to, signed\n");
                    if ((long)result <= (long)cond_val) { RET_TO_USER_ERROR }
                    break;
                case GREATER_UNSIGNED:
                    printf("greater than, unsigned\n");
                    if (result > cond_val) { RET_TO_USER_ERROR }
                    break;
                case LESS_UNSIGNED:
                    printf("less than, unsigned\n");
                    if (result < cond_val) { RET_TO_USER_ERROR }
                    break;
                case GREATER_EQUAL_UNSIGNED:
                    printf("greater than or equal to, unsigned\n");
                    if (result >= cond_val) { RET_TO_USER_ERROR }
                    break;
                case LESS_EQUAL_UNSIGNED:
                    printf("less than or equal to, unsigned\n");
                    if (result <= cond_val) { RET_TO_USER_ERROR }
                    break;
                default:
                    // illegal conditional
                    printf("... actually, illegal comparison, conditional not found\n");
                    goto EXIT;
            }
        }
        printf("\n");
    }
    EXIT:
    // probably need to do address checking again
    memcpy(result_buffer, buffer, sizeof(unsigned long) * (cur_syscall+1));
    free(buffer);
    return cur_syscall-1; // return the last successful syscall, or -1 on an initial error 
}
