#include "cond.h"

#define GET_N_CONDITIONAL(n) \
    j < bitmap.num_cond ? cond[j] = (cond_val >> (n * 4)) & mask : 0; \
    j++;

void populate_conditionals(expanded_bitmap bitmap, unsigned long **buf_ptr, unsigned char *cond) {
    unsigned char mask = 0b1111;

    for (long unsigned int j = 0; j < bitmap.num_cond;) {
        unsigned long cond_val = get_next_val((*buf_ptr));

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
}

long check_conditional(unsigned long result, unsigned long cond_val, conditional_check conditional) {
    switch (conditional) {
        case EQUAL:
            printd("equal\n");
            if (result == cond_val) { return 1; }
            break;
        case NOT_EQUAL:
            printd("not equal\n");
            if (result != cond_val) { return 1; }
            break;
        case GREATER_SIGNED:
            printd("greater than, signed\n");
            if ((long)result > (long)cond_val) { return 1; }
            break;
        case LESS_SIGNED:
            printd("less than, signed\n");
            if ((long)result < (long)cond_val) { return 1; }
            break;
        case GREATER_EQUAL_SIGNED:
            printd("greater than or equal to, signed\n");
            if ((long)result >= (long)cond_val) { return 1; }
            break;
        case LESS_EQUAL_SIGNED:
            printd("less than or equal to, signed\n");
            if ((long)result <= (long)cond_val) { return 1; }
            break;
        case GREATER_UNSIGNED:
            printd("greater than, unsigned\n");
            if (result > cond_val) { return 1; }
            break;
        case LESS_UNSIGNED:
            printd("less than, unsigned\n");
            if (result < cond_val) { return 1; }
            break;
        case GREATER_EQUAL_UNSIGNED:
            printd("greater than or equal to, unsigned\n");
            if (result >= cond_val) { return 1; }
            break;
        case LESS_EQUAL_UNSIGNED:
            printd("less than or equal to, unsigned\n");
            if (result <= cond_val) { return 1; }
            break;
        default:
            // illegal conditional
            printd("... actually, illegal comparison, conditional not found\n");
            return -1;
    }
    return 0;
}
