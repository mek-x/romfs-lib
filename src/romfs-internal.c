#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <romfs.h>
#include "romfs-internal.h"

/** private functions **/

static inline
uint32_t ReadBE32(const uint8_t *buf, size_t offset)
{
    return ((((uint32_t)*(buf + offset)     & 0xff) << 24) |
            (((uint32_t)*(buf + offset + 1) & 0xff) << 16) |
            (((uint32_t)*(buf + offset + 2) & 0xff) << 8) |
             ((uint32_t)*(buf + offset + 3) & 0xff));
}

#define LINK_FOLLOWED 1
#define LINK_NOT_FOLLOWED 0

static
int FollowHardlinks(const romfs_t *rm, uint32_t offset, uint32_t *destOffset)
{
    uint32_t next;
    nodehdr_t node;
    int      ret = LINK_NOT_FOLLOWED;

    for (int i = 0; i < ROMF_MAX_LINKS; i++) {
        if (RomfsGetNodeHdr(rm, offset, &node) != 0) return -EFAULT;

        if (!IS_TYPE(ROMFS_TYPE_HARDLINK, node.mode)) {
            *destOffset = offset;
            return ret;
        }

        // Follow the hard-link
        offset = node.info;
        ret    = LINK_FOLLOWED;
    }

    return -ELOOP;
}

/** public functions **/

int RomfsVolumeConfigure(const uint8_t *buf, volume_t *vol)
{
    if (memcmp(buf, VOLHDR_MAGIC_STR, 8) != 0) {
        return -EINVAL;
    }

    vol->size = ReadBE32(buf, VOLHDR_SIZE_OFF);
    vol->chksum = ReadBE32(buf, VOLHDR_CHKSUM_OFF);
    vol->name = (const char *)&buf[VOLHDR_VOLNAME_OFF];
    vol->rootOff = ROMFS_ALIGNUP(VOLHDR_VOLNAME_OFF + strlen(vol->name) + 1);

    return 0;
}

int RomfsGetNodeHdr(const romfs_t *rm, uint32_t offset, nodehdr_t *nd)
{
    uint8_t *buf = rm->img;

    if (offset > rm->vol.size || offset > rm->size) {
        return -EINVAL;
    }

    buf = buf + offset;

    nd->next = ReadBE32(buf, FILEHDR_NEXT_OFF) & ~(FILEHDR_NEXT_MODE_MASK);
    nd->mode = buf[FILEHDR_NEXT_OFF + 3] & 0xF;
    nd->info = ReadBE32(buf, FILEHDR_INFO_OFF);
    nd->size = ReadBE32(buf, FILEHDR_SIZE_OFF);
    nd->chksum = ReadBE32(buf, FILEHDR_CHKSUM_OFF);
    nd->name = (const char *)&buf[FILEHDR_NAME_OFF];
    nd->dataOff = offset + ROMFS_ALIGNUP(FILEHDR_NAME_OFF + strlen(nd->name) + 1);

    return 0;
}

int RomfsFindEntry(const romfs_t *rm, uint32_t startOffset, const char* path)
{
    uint32_t offset = startOffset;

    // absolute path
    if (path[0] == '/') {
        offset = rm->vol.rootOff;
    // relative path
    } else {

    }

    return 0;
}
