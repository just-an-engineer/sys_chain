#ifndef _ASM_H
#define _ASM_H

#if defined(__x86_64__)
    #define _DO_SYSCALL_USERSPACE(result_var, syscall_number, arg1, arg2, arg3, arg4, arg5, arg6) \
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

    #define _64_BIT 1


#elif defined(__aarch64__)
    #define _DO_SYSCALL_USERSPACE(result_var, syscall_number, arg1, arg2, arg3, arg4, arg5, arg6) \
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

    #define _64_BIT 1


#elif defined(__arm__)
    #define _DO_SYSCALL_USERSPACE(result_var, syscall_number, arg1, arg2, arg3, arg4, arg5, arg6) \
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

    #define _64_BIT 0


#else
    #error "Unsupported architecture."
#endif


#endif // _ASM_H