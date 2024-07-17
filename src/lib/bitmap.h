#ifndef _BITMAP_H
#define _BITMAP_H

// below sizes are in bits
#define NUM_ARG_SIZE 3
#define CONDITION_SIZE 3
#define SYMBOLIC_SIZE 6

#define NUM_ARG_SIZE_BITS ((1UL << NUM_ARG_SIZE) - 1)
#define CONDITION_SIZE_BITS ((1UL << CONDITION_SIZE) - 1)
#define SYMBOLIC_SIZE_BIT ((1UL << SYMBOLIC_SIZE) - 1)


// you still need to define the bitmap of symbolic values
#define BITMAP_CREATOR(num_args, num_conditionals, symbolic_values) \
    (((unsigned long)num_args<<(SYMBOLIC_SIZE+CONDITION_SIZE)) | \
    ((unsigned long)num_conditionals << SYMBOLIC_SIZE) | \
    symbolic_values)

typedef struct {
    unsigned long num_args;
    unsigned long num_cond;
    unsigned long sym_args;
} expanded_bitmap;

void populate_bitmap_values(expanded_bitmap *bitmap, unsigned long bitmap_val);

#endif