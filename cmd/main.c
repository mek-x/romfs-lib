#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>

#include "romfs.h"

uint8_t *romfs;
size_t romfs_size;

#define FATAL(msg, ...) { printf("Fatal: " msg "\n", ##__VA_ARGS__); return 1; }

static
int OpenRomfs(const char * filename) {
    long filesize;
    FILE *f = fopen(filename, "rb");
    if (NULL == f) FATAL("can't open %s", filename);

    fseek(f, 0L, SEEK_END);
    filesize = ftell(f);
    rewind(f);

    romfs = (uint8_t *)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fileno(f), 0);
    if (romfs == MAP_FAILED) FATAL("can't map file");

    fclose(f);

    romfs_size = filesize;

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        FATAL("Need argument with filename");
    }

    if (OpenRomfs(argv[1]) != 0) FATAL("Can't open file");

    RomfsLoad(romfs, romfs_size);


    return 0;
}
