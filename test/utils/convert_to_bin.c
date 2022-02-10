/*
This is very simple program to convert the image from basic_romfs.c to
binary file.

Keeping this for reference.
*/

#include <stdio.h>

extern unsigned char basic_romfs[];
extern unsigned int basic_romfs_len;

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;

    FILE *f = fopen(argv[1], "w");

    fwrite(basic_romfs, 1, basic_romfs_len, f);

    fclose(f);

    return 0;
}
