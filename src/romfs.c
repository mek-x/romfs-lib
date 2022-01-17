#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "romfs-internal.h"

static romfs_t romfs;

#define MAX_OPEN    20  ///> Max number of open files at once. Root is preopened
#define RESVD_FDS   3   ///> Count of reserved file descriptor numbers: stdin, stdout, stderr

#define YES 1
#define NO 0

static struct {
    uint8_t opened;
    nodehdr_t node;
} fildes[MAX_OPEN];

static
int FindFirstClosedFd()
{
    for (int i = 0; i < MAX_OPEN; i++) {
        if (!fildes[i].opened) {
            return i;
        }
    }
    return -EMFILE;
}

/* PUBLIC functions */

int RomfsLoad(uint8_t * img, size_t imgSize)
{
    int ret = 0;

    romfs.img = img;
    romfs.size = imgSize;

    ret = RomfsVolumeConfigure(romfs.img, &romfs.vol);
    if (ret != 0) return ret;

    ROMFS_TRACE("Loaded volume \"%s\". Size is %ld bytes. First entry offset = 0x%x",
        romfs.vol.name,
        romfs.vol.size,
        romfs.vol.rootOff);

    memset(&fildes, 0, sizeof(fildes));

    // preopen root dir as first file descriptor
    ret = RomfsGetNodeHdr(&romfs, romfs.vol.rootOff, &fildes[0].node);
    fildes[0].opened = YES;

    return ret;
}

int RomfsFdStat(int fd)
{
    int file = fd - 3; // subs the stdin, stdout and stderr

    if (file < 0 || file > MAX_OPEN) {
        return -EBADF;
    }

    if (!fildes[file].opened) {
        return -EBADF;
    }

    return fildes[file].node.mode;
}

int RomfsOpenAt(int fd, const char *path, int flags)
{
    int ret, f;

    fd = fd - RESVD_FDS;

    if (fd < 0) return -EBADF;

    f = FindFirstClosedFd();
    if (f < 0) return f;

    ret = RomfsFindEntry(&romfs, fildes[fd].node.off, path, &fildes[f].node);
    if (ret < 0) {
        return ret;
    }

    fildes[f].opened = YES;
    return f + RESVD_FDS; // map file descriptor to number higher than reserved fds
}

int RomfsClose(int fd)
{
    fd = fd - RESVD_FDS;

    if (fd < 0 || fd > MAX_OPEN || !fildes[fd].opened) {
        return -EBADF;
    }

    fildes[fd].opened = NO;

    return 0;
}
