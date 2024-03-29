#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef ROMFS_CUSTOM_MALLOC
#   include <stdlib.h>
#   define RomfsMalloc(x)  malloc(x)
#   define RomfsFree(x)  free(x)
#else
void *RomfsMalloc(size_t s);
void RomfsFree(void* ptr);
#endif

#ifndef PROJECT_VERSION
#   define ROMFS_VERSION "unknown"
#else
#   define ROMFS_VERSION  PROJECT_VERSION
#endif

#define ROMFS_TYPE_HARDLINK    0    ///> FILEHDR_INFO: Link destination file header
#define ROMFS_TYPE_DIRECTORY   1    ///> FILEHDR_INFO: First file's header
#define ROMFS_TYPE_FILE        2    ///> FILEHDR_INFO: Unused, must be zero
#define ROMFS_TYPE_SOFTLINK    3    ///> FILEHDR_INFO: Unused, must be zero
#define ROMFS_TYPE_BLOCKDEV    4    ///> FILEHDR_INFO: 16/16 bits major/minor number
#define ROMFS_TYPE_CHARDEV     5    ///> FILEHDR_INFO: 16/16 bits major/minor number
#define ROMFS_TYPE_SOCKET      6    ///> FILEHDR_INFO: Unused, must be zero
#define ROMFS_TYPE_FIFO        7    ///> FILEHDR_INFO: Unused, must be zero
#define ROMFS_TYPE_MASK        0x7
#define ROMFS_MODE_EXEC        8    ///> Modifier for TYPE_DIRECTORY and TYPE_FILE

#define IS_TYPE(type,mode)     (((mode)&ROMFS_TYPE_MASK) == (type))
#define IS_HARDLINK(mode)      IS_TYPE(ROMFS_TYPE_HARDLINK, (mode))
#define IS_DIRECTORY(mode)     IS_TYPE(ROMFS_TYPE_DIRECTORY, (mode))
#define IS_FILE(mode)          IS_TYPE(ROMFS_TYPE_FILE, (mode))
#define IS_EXEC(mode)          (((mode)&(~ROMFS_TYPE_MASK)) == ROMFS_MODE_EXEC)

#define ROMFS_O_FLAGS_NONBLOCK  (1 << 0)

#define ROMFS_COOKIE_START      0
#define ROMFS_COOKIE_LAST       0xFFFFFFFF

typedef struct {
    uint32_t ino;
    uint32_t size;
    uint32_t chksum;
    uint8_t  mode;
} romfs_stat_t;

typedef enum {
    ROMFS_SEEK_SET,
    ROMFS_SEEK_CUR,
    ROMFS_SEEK_END,
} romfs_seek_t;

typedef struct {
    uint32_t    next;
    uint32_t    inode;
    size_t      nameLen;
    uint8_t     type;
    const char  *name;
} romfs_dirent_t;

typedef struct romfs_t *romfs_t;

int RomfsLoad(uint8_t * img, size_t imgSize, romfs_t *romfs);
void RomfsUnload(romfs_t *romfs);
int RomfsOpenAt(romfs_t t, int fd, const char *path, int flags);
int RomfsOpenRoot(romfs_t t, const char *path, int flags);
int RomfsClose(romfs_t t, int fd);
int RomfsFdStat(romfs_t t, int fd, romfs_stat_t *stat);
int RomfsFdStatAt(romfs_t t, int fd, const char *path, romfs_stat_t *stat);
int RomfsRead(romfs_t t, int fd, void *buf, size_t nbyte);
int RomfsSeek(romfs_t t, int fd, long off, romfs_seek_t whence);
int RomfsTell(romfs_t t, int fd, long *off);
int RomfsReadDir(romfs_t t, int fd, romfs_dirent_t *buf, size_t bufLen, uint32_t *cookie, size_t *bufUsed);
int RomfsMapFile(romfs_t t, void **addr, size_t *len, int fd, uint32_t off);
