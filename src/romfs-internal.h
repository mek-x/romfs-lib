#pragma once

#include <romfs.h>

#if DEBUG
#   include <stdio.h>
#   define ROMFS_TRACE(fmt, ...) fprintf(stderr, "[%s:%d]: " fmt "\n",__FUNCTION__ ,__LINE__ , ##__VA_ARGS__)
#else
#   define ROMFS_TRACE(fmt, ...)
#endif

#define MAX_OPEN    20  ///> Max number of open files at once. Root is preopened

// Volume header

#define VOLHDR_MAGIC_OFF      0   ///>  0-7:  ROMFS magic.
#define VOLHDR_SIZE_OFF       8   ///>  8-11: The number of accessible bytes in this fs.
#define VOLHDR_CHKSUM_OFF     12  ///> 12-15: The checksum of the FIRST 512 BYTES.
#define VOLHDR_VOLNAME_OFF    16  ///> 16-*:  The zero terminated name of the volume, padded to 16 byte boundary.

#define VOLHDR_MAGIC_STR      "-rom1fs-" ///> Magic ASCII string.

// File header

#define FILEHDR_NEXT_OFF      0   ///>  0-3:  The offset of the next file header (zero if no more files).
#define FILEHDR_INFO_OFF      4   ///>  4-7:  Info for directories/hard links/devices.
#define FILEHDR_SIZE_OFF      8   ///>  8-11:  The size of this file in bytes.
#define FILEHDR_CHKSUM_OFF    12  ///>  12-15: Covering the meta data, including the file name, and padding.
#define FILEHDR_NAME_OFF      16  ///>  16-..: The zero terminated name of the file, padded to 16 byte boundary.

#define FILEHDR_NEXT_MODE_MASK  0x0F    ///> Mask for mode/type field

// Alignment

#define ROMFS_ALIGNMENT       16
#define ROMFS_MAXPADDING      (ROMFS_ALIGNMENT-1)
#define ROMFS_ALIGNMASK       (~ROMFS_MAXPADDING)
#define ROMFS_ALIGNUP(addr)   ((((uint32_t)(addr))+ROMFS_MAXPADDING)&ROMFS_ALIGNMASK)
#define ROMFS_ALIGNDOWN(addr) (((uint32_t)(addr))&ROMFS_ALIGNMASK)

#define ROMF_MAX_LINKS        16

typedef struct {
    uint32_t off;
    uint32_t next;
    uint32_t info;
    uint32_t size;
    uint32_t chksum;
    const char *name;
    uint32_t dataOff;
    uint8_t mode;
} nodehdr_t;

typedef struct fildes_t {
    uint8_t     opened;
    nodehdr_t   node;
    void        *cur;
} fildes_t;

typedef struct {
    size_t size;
    const char *name;
    uint32_t chksum;
    uint32_t rootOff;
} volume_t;

struct romfs_t {
    uint8_t *img;
    size_t size;
    volume_t vol;
    fildes_t fildes[MAX_OPEN];
};

int RomfsVolumeConfigure(const uint8_t *buf, volume_t *vol);
int RomfsGetNodeHdr(const struct romfs_t *rm, uint32_t offset, nodehdr_t *nd);
int RomfsSearchDir(const struct romfs_t *rm, const char *name, uint32_t *offset);
int RomfsFindEntry(const struct romfs_t *rm, uint32_t startOffset, const char* path, nodehdr_t *nd);
