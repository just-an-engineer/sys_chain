#ifndef _IO_H
#define _IO_H


#ifdef DEBUG
    #include <stdio.h>
    #define printd(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define printd(fmt, ...)
#endif

#endif // _IO_H