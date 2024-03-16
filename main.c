#include "syscall.h"

#include <stdio.h>
#include <stdlib.h>
// #include <sys/types.h>
// #include <sys/stat.h>
#include <fcntl.h>


// still need to check conditionals fully


void open_error(long error) {
    printf("open error: %ld\n", error);
    exit(1);
}
void read_error(long error) {
    printf("read error: %ld\n", error);
    exit(1);
}
void write_error(long error) {
    printf("write error: %ld\n", error);
    exit(1);
}
void close_error(long error) {
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

    long syscall[] = {
        2, // syscall open
        (3<<16) + (1 << 8), // bitmap
        0x0, // mode
        O_RDWR, // flags
        (long)filename, // filename
        (long)open_error, // error function
        LESS, // condition
        0,  // condition value

        0, // syscall read
        (3<<16) + (1 << 8) + 0b1, // bitmap
        15, // count
        (long)buffer, // buffer
        0, // file descriptor, symbolic
        (long)read_error, // error function
        LESS, // condition
        0,  // condition value

        1, // syscall write
        (3<<16) + (1 << 8) + 0b1, // bitmap
        8, // count
        (long)replace_test, // buffer
        0, // file descriptor, symbolic
        (long)write_error, // error function
        LESS, // condition
        0,  // condition value

        3, // syscall close
        (1<<16) + (1 << 8) + 0b1, // bitmap
        0, // file descriptor, symbolic
        (long)close_error, // error function
        LESS, // condition
        0,  // condition value
    };

    syscall_negative_one(sizeof(syscall)/sizeof(long), syscall);
    printf("Ran syscall\n");
}