#pragma once

#include <stddef.h>
#include <stdint.h>

#define ROMFS_MAX_NAME_LEN     16
#define ROMFS_MAX_PATH_LEN     256

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

int RomfsLoad(uint8_t * img, size_t imgSize);
int RomfsFdStat(int fd);

#define ROMFS_O_FLAGS_NONBLOCK  (1 << 0)

int RomfsOpenAt(int fd, const char *path, int flags);
int RomfsClose(int fd);
