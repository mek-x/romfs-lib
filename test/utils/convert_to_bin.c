/*
This is very simple program to convert the image from basic_romfs.c to
binary file.

Keeping this for reference.
*/

#include <stdio.h>

#ifndef ROMFS
#   define ROMFS basic_romfs
#endif

#define XLEN(x) x ## _len
#define LEN(x) XLEN(x)

extern unsigned char ROMFS[];
extern unsigned int LEN(ROMFS);

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;

    FILE *f = fopen(argv[1], "w");

    fwrite(ROMFS, 1, LEN(ROMFS), f);

    fclose(f);

    return 0;
}
