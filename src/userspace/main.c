#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "../userspace/syscall.h"

// still need to check conditionals fully

void open_error(unsigned long error) {
    printd("open error: %ld\n", error);
    exit(1);
}
void read_error(unsigned long error) {
    printd("read error: %ld\n", error);
    exit(1);
}
void write_error(unsigned long error) {
    printd("write error: %ld\n", error);
    exit(1);
}
void close_error(unsigned long error) {
    printd("close error: %ld\n", error);
    exit(1);
}

int main() {
    char filename[] = "test.txt";
    printd("Filename address: %p\n", filename);
    char replace_test[] = "replace!";
    printd("Replace_test address: %p\n", replace_test);
    char *buffer = malloc(16);
    printd("Buffer address: %p\n\n", buffer);
    unsigned long result_buf[4];
    // print address of all error functions

    unsigned long syscall[] = {
        2, // syscall open
        BITMAP_CREATOR(3, 1, 0), // bitmap
        (unsigned long)filename, // filename
        O_RDWR, // flags
        0x0, // mode
        (unsigned long)open_error, // error function
        LESS_SIGNED, // condition
        0,  // condition value

        0, // syscall read
        BITMAP_CREATOR(3, 1, 0b1), // bitmap
        0, // file descriptor, symbolic
        (unsigned long)buffer, // buffer
        15, // count
        (unsigned long)read_error, // error function
        LESS_SIGNED, // condition
        0,  // condition value

        1, // syscall write
        BITMAP_CREATOR(3, 1, 0b1), // bitmap
        0, // file descriptor, symbolic
        (unsigned long)replace_test, // buffer
        8, // count
        (unsigned long)write_error, // error function
        LESS_SIGNED, // condition
        0,  // condition value

        3, // syscall close
        BITMAP_CREATOR(1, 1, 0b1), // bitmap
        0, // file descriptor, symbolic
        (unsigned long)close_error, // error function
        LESS_SIGNED, // condition
        0,  // condition value
    };

    syscall_chain(sizeof(syscall)/sizeof(unsigned long), syscall, result_buf);
    
    printd("Ran syscall\n");
    printd("Buffer contains %s\n", buffer);
    for (int i=0; i<4; i++) {
        printd("Syscall %d gave result %ld\n", i, result_buf[i]);
    }
}