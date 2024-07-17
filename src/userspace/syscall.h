#ifndef SYSCALL_H
#define SYSCALL_H

#define USERSPACE
#define SYS_CHAIN_NUM 500

#include <stdlib.h>
#include "../lib/io.h"
#include "../lib/bitmap.h"
#include "../lib/cond.h"

unsigned long syscall_chain(unsigned long size, unsigned long* address, unsigned long* result_buffer);

#endif