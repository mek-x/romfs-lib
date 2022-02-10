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

#define ABS(x)  ((x) < 0 ? -(x) : (x))

static struct {
    uint8_t     opened;
    nodehdr_t   node;
    void        *cur;
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
    ROMFS_TRACE("Romfs lib, v.%s", ROMFS_VERSION);
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
    fildes[0].cur = (void *)(romfs.img + fildes[0].node.dataOff);

    return ret;
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
    fildes[f].cur = (void *)(romfs.img + fildes[f].node.dataOff);

    return f + RESVD_FDS; // map file descriptor to number higher than reserved fds
}

int RomfsOpenRoot(const char *path, int flags) {
    return RomfsOpenAt(RESVD_FDS, path, flags);
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

int RomfsFdStat(int fd, romfs_stat_t *stat)
{
    fd = fd - RESVD_FDS;

    if (fd < 0 || fd > MAX_OPEN || !fildes[fd].opened) {
        return -EBADF;
    }

    if (stat != NULL) {
        stat->ino    = fildes[fd].node.off;
        stat->chksum = fildes[fd].node.chksum;
        stat->size   = fildes[fd].node.size;
        stat->mode   = fildes[fd].node.mode;
    }

    return fildes[fd].node.mode;
}

int RomfsFdStatAt(int fd, const char *path, romfs_stat_t *stat) {
    int ret;
    nodehdr_t node;

    fd = fd - RESVD_FDS;

   if (fd < 0 || fd > MAX_OPEN || !fildes[fd].opened) {
        return -EBADF;
    }

    ret = RomfsFindEntry(&romfs, fildes[fd].node.off, path, &node);
    if (ret < 0) {
        return ret;
    }

    if (stat != NULL) {
        stat->ino    = node.off;
        stat->chksum = node.chksum;
        stat->size   = node.size;
        stat->mode   = node.mode;
    }

    return node.mode;
}

int RomfsRead(int fd, void *buf, size_t nbyte)
{
    size_t toRead;

    fd = fd - RESVD_FDS;

    if (buf == NULL) {
        return -EINVAL;
    }

    if (fd < 0 || fd > MAX_OPEN || !fildes[fd].opened) {
        return -EBADF;
    }

    if (IS_DIRECTORY(fildes[fd].node.mode)) {
        return -EISDIR;
    }

    toRead = (unsigned long)(romfs.img + fildes[fd].node.dataOff + fildes[fd].node.size) - (unsigned long)fildes[fd].cur;
    if (nbyte > toRead) {
        nbyte = toRead;
    }

    if (nbyte == 0) {
        return 0;
    }

    memcpy(buf, fildes[fd].cur, nbyte);

    fildes[fd].cur += nbyte;

    return nbyte;
}

int RomfsSeek(int fd, long off, romfs_seek_t whence)
{
    fd = fd - RESVD_FDS;

    if (fd < 0 || fd > MAX_OPEN || !fildes[fd].opened) {
        return -EBADF;
    }

    if (!IS_FILE(fildes[fd].node.mode)) {
        return -EBADF;
    }

    if (ABS(off) > fildes[fd].node.size) {
        return -EINVAL;
    }

    ROMFS_TRACE("%p", fildes[fd].cur);

    switch (whence)
    {
    case ROMFS_SEEK_SET:
        if (off < 0) {
            return -EINVAL;
        }
        fildes[fd].cur = (void *)(romfs.img + (fildes[fd].node.dataOff + off));
        break;
    case ROMFS_SEEK_CUR:
        ROMFS_TRACE("%ld %p == %p --> %p", off, fildes[fd].cur + off,  romfs.img + fildes[fd].node.dataOff, romfs.img + fildes[fd].node.dataOff + fildes[fd].node.size);
        if ( (fildes[fd].cur + off > (void *)(romfs.img + fildes[fd].node.dataOff + fildes[fd].node.size)) ||
             (fildes[fd].cur + off < (void *)(romfs.img + fildes[fd].node.dataOff)) ) {
            return -EINVAL;
        }
        fildes[fd].cur += off;
        break;
    case ROMFS_SEEK_END:
        if (off > 0) {
            return -EINVAL;
        }
        fildes[fd].cur = (void *)(romfs.img + (fildes[fd].node.dataOff + fildes[fd].node.size + off));
        break;
    default:
        return -EINVAL;
        break;
    }

    return 0;
}

int RomfsTell(int fd, long *off)
{
    fd = fd - RESVD_FDS;

    if (NULL == off) {
        return -EINVAL;
    }

    if (fd < 0 || fd > MAX_OPEN || !fildes[fd].opened) {
        return -EBADF;
    }

    if (!IS_FILE(fildes[fd].node.mode)) {
        return -EBADF;
    }

    *off = (long)(fildes[fd].cur - (void *)(romfs.img + fildes[fd].node.dataOff));

    return 0;
}

/* TODO:
    - follow hardlinks
    - cookie can be bad
*/
int RomfsReadDir(int fd, romfs_dirent_t *buf, size_t bufLen, uint32_t *cookie, size_t *bufUsed)
{
    nodehdr_t curNode;
    int ret;

    fd = fd - RESVD_FDS;

    if (buf == NULL || bufUsed == NULL || cookie == NULL) {
        return -EINVAL;
    }

    if (fd < 0 || fd > MAX_OPEN || !fildes[fd].opened) {
        return -EBADF;
    }

    if (!IS_DIRECTORY(fildes[fd].node.mode)) {
        return -ENOTDIR;
    }

    if (*cookie == ROMFS_COOKIE_LAST) {
        *bufUsed = 0;
        return 0;
    }

    if (*cookie == 0) {
        *cookie = fildes[fd].node.info;
    }

    ret = RomfsGetNodeHdr(&romfs, *cookie, &curNode);
    if (ret < 0) {
        return -EINVAL;
    }

    for (*bufUsed = 0; *bufUsed < bufLen; (*bufUsed)++) {
        buf[*bufUsed].name    = curNode.name;
        buf[*bufUsed].nameLen = strlen(curNode.name);
        buf[*bufUsed].inode   = curNode.off;
        buf[*bufUsed].next    = curNode.next;
        buf[*bufUsed].type    = curNode.mode;

        if (curNode.next) {
            *cookie = curNode.next;

            ret = RomfsGetNodeHdr(&romfs, curNode.next, &curNode);
            if (ret < 0) {
                break;
            }
        }
        else {
            *cookie = ROMFS_COOKIE_LAST;
            (*bufUsed)++;
            break;
        }
    }

    ROMFS_TRACE("last cookie = 0x%x", *cookie);

    return 0;
}

int RomfsMapFile(void **addr, size_t *len, int fd, uint32_t off)
{
    fd = fd - RESVD_FDS;

    if (fd < 0 || fd > MAX_OPEN || !fildes[fd].opened) {
        return -EBADF;
    }

    if (!IS_FILE(fildes[fd].node.mode)) {
        return -EACCES;
    }

    if (off >= fildes[fd].node.size) {
        return -EINVAL;
    }

    *addr = romfs.img + (fildes[fd].node.dataOff + off);
    *len = fildes[fd].node.size - off;

    return 0;
}
