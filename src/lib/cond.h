#ifndef _COND_H
#define _COND_H

#include "io.h"
#include "lib.h"
#include "bitmap.h"

typedef enum {
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
} conditional_check;

void populate_conditionals(expanded_bitmap bitmap, unsigned long **ptr, unsigned char *cond);
long check_conditional(unsigned long result, unsigned long cond_val, conditional_check conditional);

#endif // _COND_H