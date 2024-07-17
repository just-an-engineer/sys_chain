#include "bitmap.h"

void populate_bitmap_values(expanded_bitmap *bitmap, unsigned long bitmap_val) {
    bitmap->sym_args = bitmap_val & SYMBOLIC_SIZE_BIT;
    bitmap->num_cond = (bitmap_val >> SYMBOLIC_SIZE) & CONDITION_SIZE_BITS;
    bitmap->num_args = (bitmap_val >> (SYMBOLIC_SIZE+CONDITION_SIZE)) & NUM_ARG_SIZE_BITS;
}
