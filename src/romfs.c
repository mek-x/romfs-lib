#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "romfs-internal.h"


#define RESVD_FDS   3   ///> Count of reserved file descriptor numbers: stdin, stdout, stderr

#define YES 1
#define NO 0

#define ABS(x)  ((x) < 0 ? -(x) : (x))

static
int FindFirstClosedFd(fildes_t *fildes)
{
    for (int i = 0; i < MAX_OPEN; i++) {
        if (!fildes[i].opened) {
            return i;
        }
    }
    return -EMFILE;
}

/* PUBLIC functions */

int RomfsLoad(uint8_t * img, size_t imgSize, romfs_t *rom)
{
    int ret = 0;
    ROMFS_TRACE("Romfs lib, v.%s", ROMFS_VERSION);

    if (NULL == rom) return -EINVAL;

    *rom = (romfs_t)RomfsMalloc(sizeof(struct romfs_t));
    if (NULL == *rom) return -ENOMEM;

    struct romfs_t *r = *rom;

    r->img = img;
    r->size = imgSize;

    ret = RomfsVolumeConfigure(r->img, &r->vol);
    if (ret != 0) { RomfsUnload(rom); return ret; }

    ROMFS_TRACE("Loaded volume \"%s\". Size is %ld bytes. First entry offset = 0x%x",
        r->vol.name,
        r->vol.size,
        r->vol.rootOff);

    memset(r->fildes, 0, sizeof(r->fildes));

    // preopen root dir as first file descriptor
    ret = RomfsGetNodeHdr((const struct romfs_t *)r, r->vol.rootOff, &r->fildes[0].node);
    if (ret != 0) { RomfsUnload(rom); return ret; }

    r->fildes[0].opened = YES;
    r->fildes[0].cur = (void *)(r->img + r->fildes[0].node.dataOff);

    return ret;
}

void RomfsUnload(romfs_t *romfs)
{
    if (NULL != romfs) RomfsFree(*romfs);
    *romfs = NULL;
}

int RomfsOpenAt(romfs_t t, int fd, const char *path, int flags)
{
    int ret, f;

    fd = fd - RESVD_FDS;

    if (NULL == t) return -EINVAL;

    if (fd < 0) return -EBADF;

    f = FindFirstClosedFd(t->fildes);
    if (f < 0) return f;

    ret = RomfsFindEntry(t, t->fildes[fd].node.off, path, &t->fildes[f].node);
    if (ret < 0) {
        return ret;
    }

    t->fildes[f].opened = YES;
    t->fildes[f].cur = (void *)(t->img + t->fildes[f].node.dataOff);

    return f + RESVD_FDS; // map file descriptor to number higher than reserved fds
}

int RomfsOpenRoot(romfs_t t, const char *path, int flags) {
    return RomfsOpenAt(t, RESVD_FDS, path, flags);
}

int RomfsClose(romfs_t t, int fd)
{
    fd = fd - RESVD_FDS;

    if (fd < 0 || fd > MAX_OPEN || !t->fildes[fd].opened) {
        return -EBADF;
    }

    t->fildes[fd].opened = NO;

    return 0;
}

int RomfsFdStat(romfs_t t, int fd, romfs_stat_t *stat)
{
    fd = fd - RESVD_FDS;

    if (fd < 0 || fd > MAX_OPEN || !t->fildes[fd].opened) {
        return -EBADF;
    }

    if (stat != NULL) {
        stat->ino    = t->fildes[fd].node.off;
        stat->chksum = t->fildes[fd].node.chksum;
        stat->size   = t->fildes[fd].node.size;
        stat->mode   = t->fildes[fd].node.mode;
    }

    return t->fildes[fd].node.mode;
}

int RomfsFdStatAt(romfs_t t, int fd, const char *path, romfs_stat_t *stat) {
    int ret;
    nodehdr_t node;

    if (NULL == t) return -EINVAL;

    fd = fd - RESVD_FDS;
    if (fd < 0 || fd > MAX_OPEN || !t->fildes[fd].opened) {
        return -EBADF;
    }

    ret = RomfsFindEntry(t, t->fildes[fd].node.off, path, &node);
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

int RomfsRead(romfs_t t, int fd, void *buf, size_t nbyte)
{
    size_t toRead;

    if (NULL == t) return -EINVAL;

    if (buf == NULL) {
        return -EINVAL;
    }

    fd = fd - RESVD_FDS;
    if (fd < 0 || fd > MAX_OPEN || !t->fildes[fd].opened) {
        return -EBADF;
    }

    if (IS_DIRECTORY(t->fildes[fd].node.mode)) {
        return -EISDIR;
    }

    toRead = (unsigned long)(t->img + t->fildes[fd].node.dataOff + t->fildes[fd].node.size) - (unsigned long)t->fildes[fd].cur;
    if (nbyte > toRead) {
        nbyte = toRead;
    }

    if (nbyte == 0) {
        return 0;
    }

    memcpy(buf, t->fildes[fd].cur, nbyte);

    t->fildes[fd].cur += nbyte;

    return nbyte;
}

int RomfsSeek(romfs_t t, int fd, long off, romfs_seek_t whence)
{
    if (NULL == t) return -EINVAL;

    fd = fd - RESVD_FDS;
    if (fd < 0 || fd > MAX_OPEN || !t->fildes[fd].opened) {
        return -EBADF;
    }

    if (!IS_FILE(t->fildes[fd].node.mode)) {
        return -EBADF;
    }

    if (ABS(off) > t->fildes[fd].node.size) {
        return -EINVAL;
    }

    ROMFS_TRACE("%p", t->fildes[fd].cur);

    switch (whence)
    {
    case ROMFS_SEEK_SET:
        if (off < 0) {
            return -EINVAL;
        }
        t->fildes[fd].cur = (void *)(t->img + (t->fildes[fd].node.dataOff + off));
        break;
    case ROMFS_SEEK_CUR:
        ROMFS_TRACE("%ld %p == %p --> %p", off, fildes[fd].cur + off,  romfs.img + fildes[fd].node.dataOff, romfs.img + fildes[fd].node.dataOff + fildes[fd].node.size);
        if ( (t->fildes[fd].cur + off > (void *)(t->img + t->fildes[fd].node.dataOff + t->fildes[fd].node.size)) ||
             (t->fildes[fd].cur + off < (void *)(t->img + t->fildes[fd].node.dataOff)) ) {
            return -EINVAL;
        }
        t->fildes[fd].cur += off;
        break;
    case ROMFS_SEEK_END:
        if (off > 0) {
            return -EINVAL;
        }
        t->fildes[fd].cur = (void *)(t->img + (t->fildes[fd].node.dataOff + t->fildes[fd].node.size + off));
        break;
    default:
        return -EINVAL;
        break;
    }

    return 0;
}

int RomfsTell(romfs_t t, int fd, long *off)
{
    if (NULL == t) return -EINVAL;

    if (NULL == off) {
        return -EINVAL;
    }

    fd = fd - RESVD_FDS;
    if (fd < 0 || fd > MAX_OPEN || !t->fildes[fd].opened) {
        return -EBADF;
    }

    if (!IS_FILE(t->fildes[fd].node.mode)) {
        return -EBADF;
    }

    *off = (long)(t->fildes[fd].cur - (void *)(t->img + t->fildes[fd].node.dataOff));

    return 0;
}

/* TODO:
    - follow hardlinks
    - cookie can be bad
*/
int RomfsReadDir(romfs_t t, int fd, romfs_dirent_t *buf, size_t bufLen, uint32_t *cookie, size_t *bufUsed)
{
    nodehdr_t curNode;
    int ret;

    if (NULL == t) return -EINVAL;

    if (buf == NULL || bufUsed == NULL || cookie == NULL) {
        return -EINVAL;
    }

    fd = fd - RESVD_FDS;
    if (fd < 0 || fd > MAX_OPEN || !t->fildes[fd].opened) {
        return -EBADF;
    }

    if (!IS_DIRECTORY(t->fildes[fd].node.mode)) {
        return -ENOTDIR;
    }

    if (*cookie == ROMFS_COOKIE_LAST) {
        *bufUsed = 0;
        return 0;
    }

    if (*cookie == 0) {
        *cookie = t->fildes[fd].node.info;
    }

    ret = RomfsGetNodeHdr(t, *cookie, &curNode);
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

            ret = RomfsGetNodeHdr(t, curNode.next, &curNode);
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

int RomfsMapFile(romfs_t t, void **addr, size_t *len, int fd, uint32_t off)
{
    if (NULL == t) return -EINVAL;

    if (NULL == addr || NULL == len) {
        return -EINVAL;
    }

    fd = fd - RESVD_FDS;
    if (fd < 0 || fd > MAX_OPEN || !t->fildes[fd].opened) {
        return -EBADF;
    }

    if (!IS_FILE(t->fildes[fd].node.mode)) {
        return -EACCES;
    }

    if (off >= t->fildes[fd].node.size) {
        return -EINVAL;
    }

    *addr = t->img + (t->fildes[fd].node.dataOff + off);
    *len = t->fildes[fd].node.size - off;

    return 0;
}
