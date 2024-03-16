#include <stdio.h>
#include <stdlib.h>
// #include <sys/types.h>
// #include <sys/stat.h>
#include <fcntl.h>
#include "syscall.h"

// still need to check conditionals fully

void open_error(unsigned long error) {
    printf("open error: %ld\n", error);
    exit(1);
}
void read_error(unsigned long error) {
    printf("read error: %ld\n", error);
    exit(1);
}
void write_error(unsigned long error) {
    printf("write error: %ld\n", error);
    exit(1);
}
void close_error(unsigned long error) {
    printf("close error: %ld\n", error);
    exit(1);
}

int main() {
    char filename[] = "test.txt";
    printf("filename address: %p\n", filename);
    char replace_test[] = "replace!";
    printf("replace_test address: %p\n", replace_test);
    char *buffer = malloc(16);
    printf("buffer address: %p\n\n", buffer);
    unsigned long result_buf[4];

    unsigned long syscall[] = {
        2, // syscall open
        (3<<16) + (1 << 8), // bitmap
        0x0, // mode
        O_RDWR, // flags
        (unsigned long)filename, // filename
        (unsigned long)open_error, // error function
        LESS, // condition
        0,  // condition value

        0, // syscall read
        (3<<16) + (1 << 8) + 0b1, // bitmap
        15, // count
        (unsigned long)buffer, // buffer
        0, // file descriptor, symbolic
        (unsigned long)read_error, // error function
        LESS, // condition
        0,  // condition value

        1, // syscall write
        (3<<16) + (1 << 8) + 0b1, // bitmap
        8, // count
        (unsigned long)replace_test, // buffer
        0, // file descriptor, symbolic
        (unsigned long)write_error, // error function
        LESS, // condition
        0,  // condition value

        3, // syscall close
        (1<<16) + (1 << 8) + 0b1, // bitmap
        0, // file descriptor, symbolic
        (unsigned long)close_error, // error function
        LESS, // condition
        0,  // condition value
    };

    syscall_negative_one(sizeof(syscall)/sizeof(unsigned long), syscall, result_buf);
    printf("Ran syscall\n");
    printf("buffer contains %s\n", buffer);
    for (int i=0; i<4; i++) {
        printf("Syscall %d gave result %ld\n", i, result_buf[i]);
    }
}