// struct layout:
// long syscall
// long bitmap // [symbolic arguments][num of conditionals][number of arguments]
    // in this case we just use a char for each arg, and leave the remaining 5 bytes on 64 bit systems for future use
    // in reality, this can be optimized for each segment
    // the bitmap will likely be architecture size dependent
// long args
// ...
    // the args need to be in reverse order, for ease of programming, currently
// error function address
// conditional enums, 2 per byte, no spilling between bytes. There is 1 long for every 8 conditionals
// conditional values to compare against

// #define x86_64
#include "syscall.h"


#define SET_REGISTER(n, register) \
    if ((sym_args >> n) & 1) { \
        if (*(buf_ptr) > size) { return -1; } \
        value = buffer[*(buf_ptr)]; \
        printf("Symbolic value detected, setting register %d to 0x%lx, result of %ld\n", n, value, *(buf_ptr)); \
    } else { \
        value = *(buf_ptr); \
        printf("Setting register %d to 0x%lx\n", n, value); \
    } \
    register = value; \
    num_args--; \
    buf_ptr++;

#ifdef __x86_64__
    #define SYSCALL asm("syscall");
#elif defined(__aarch64__)
    #define SYSCALL asm("svc 0");
#elif defined(__arm__)
    #define SYSCALL asm("swi 0");
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


long syscall_negative_one(long size, long* address) {
    long buffer[size]; // likely need to actually place on the heap due to potential size
    // pointer checking
    memcpy(buffer, address, sizeof(long) * size);

    long result_buffer[size];

    long *buf_ptr = buffer;
    for (long i = 0; i < size; i++) {
        long syscall = *(buf_ptr);      buf_ptr++;
        long bitmap = *(buf_ptr);       buf_ptr++;
        unsigned char sym_args = bitmap & 0xFF;
        unsigned char num_cond = (bitmap >> 8) & 0xFF;
        unsigned char num_args = (bitmap >> 16) & 0xFF;

        printf("Running syscall %ld with %d args and %d conditionals\n", syscall, num_args, num_cond);

        long r0, r1, r2, r3, r4, r5;
        
        while (num_args > 0) {
            long value;
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
                    return -1;
            }
        }

        // this is where we would jump to the syscall function in the syscall table
        // #define DO_SYSCALL(result_var, syscall_number, arg1, arg2, arg3, arg4, arg5, arg6)
        printf("Running syscall %ld with args:\n\tr0: 0x%lx\n\tr1:0x%lx\n\tr2:0x%lx\n\tr3:0x%lx\n\tr4:0x%lx\n\tr50x%lx\n", syscall, r0, r1, r2, r3, r4, r5);
        long result;
        DO_SYSCALL(result, syscall, r0, r1, r2, r3, r4, r5);
        buffer[i] = result;
        printf("Result: 0x%lx, saved to %ld\n", result, i);
        
        // function pointer
        void (*error_func)(long) = (void (*)(long int))*(buf_ptr);
        buf_ptr++;

        short cond[num_cond];

        for (int j = 0; j < num_cond;) {
            long cond_val = *(buf_ptr);
            buf_ptr++;
            j<num_cond ? cond[j] = cond_val & 0xFF : 0;
            j++;
            j<num_cond ? cond[j] = (cond_val >> sizeof(short)) & 0xFFFF : 0;
            j++;
        }

        for (int j = 0; j < num_cond; j++) {
            long cond_val = *(buf_ptr);     buf_ptr++;
            switch (cond[j]) {
                // this is where we would overwrite the rip or instruction pointer in the saved context, and return to userspace
                case EQUAL:
                    if (result == cond_val) { error_func(result); }
                    break;
                case NOT_EQUAL:
                    if (result != cond_val) { error_func(result); }
                    break;
                case GREATER:
                    if (result > cond_val) { error_func(result); }
                    break;
                case LESS:
                    if (result < cond_val) { error_func(result); }
                    break;
                case GREATER_EQUAL:
                    if (result >= cond_val) { error_func(result); }
                    break;
                case LESS_EQUAL:
                    if (result <= cond_val) { error_func(result); }
                    break;
                default:
                    // illegal conditional
                    return -1;
            }
        }
        printf("\n");
    }
}
