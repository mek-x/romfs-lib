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

    nd->off = offset;
    nd->next = ReadBE32(buf, FILEHDR_NEXT_OFF) & ~(FILEHDR_NEXT_MODE_MASK);
    nd->mode = buf[FILEHDR_NEXT_OFF + 3] & 0xF;
    nd->info = ReadBE32(buf, FILEHDR_INFO_OFF);
    nd->size = ReadBE32(buf, FILEHDR_SIZE_OFF);
    nd->chksum = ReadBE32(buf, FILEHDR_CHKSUM_OFF);
    nd->name = (const char *)&buf[FILEHDR_NAME_OFF];
    nd->dataOff = offset + ROMFS_ALIGNUP(FILEHDR_NAME_OFF + strlen(nd->name) + 1);

    return 0;
}

int RomfsSearchDir(const romfs_t *rm, const char *name, uint32_t *offset)
{
    int ret;
    nodehdr_t node;
    uint32_t off = *offset;

    while (off != 0) {
        ret = RomfsGetNodeHdr(rm, off, &node);
        if (ret) return -EINVAL;

        if (strcmp(node.name, name) == 0) {
            *offset = off;
            return 0;
    }

        off = node.next;
    }

    return -ENOENT;
}

int RomfsParsePath(const char *path, filename_t entryList[], size_t entryListLen)
{
    int ret = 0;
    path_t pathCopy;
    char *p;

    if (entryList == NULL) {
        // we're just counting depth, use some arbitraty big number
        entryListLen = 999;
    }

    if (entryListLen == 0 && entryList != NULL) {
        return -EINVAL;
    }

    if (strnlen(path, ROMFS_MAX_PATH_LEN) == ROMFS_MAX_PATH_LEN) {
        return -ENAMETOOLONG;
    }

    strcpy(pathCopy, path);
    p = pathCopy;
    if (entryList != NULL) {
        entryList[0][0] = '\0';
    }

    // remove initial / and set first element to indicate absolute path
    if (*p == '/') {
        if (entryList != NULL) {
            strcpy(entryList[0], "/");
        }
        ret = 1;
        while (*p == '/') {p++;}
    }

    p = strtok(p, "/");
    while (ret < entryListLen) {
        if (p != NULL) {
            if (strnlen(p, ROMFS_MAX_NAME_LEN) == ROMFS_MAX_NAME_LEN) {
                ret = -ENAMETOOLONG;
                break;
            }
            if (entryList != NULL) {
                strcpy(entryList[ret], p);
            }
            ret++;
        } else break;
        p = strtok(NULL, "/");
    }

    return ret;
}

int RomfsFindEntry(const romfs_t *rm, uint32_t offset, const char* path, nodehdr_t *nd)
{
    int d, ret;
    filename_t pathList[10]; // what should be max depth? Maybe need to reengineer this...

    d = RomfsParsePath(path, pathList, 10);
    if (d < 0) return d;

    for (int i = 0; i < d; i++) {
        if (pathList[i][0] == '/') { //special case for root dir
            offset = rm->vol.rootOff;
        } else {
            ret = RomfsSearchDir(rm, pathList[i], &offset);
            ROMFS_TRACE("%s -> %d (%x)", pathList[i], ret, offset);
            if (ret < 0) {
                return ret;
            }
        }
        ret = RomfsGetNodeHdr(rm, offset, nd);
        if (ret < 0) {
            return ret;
        }

        ROMFS_TRACE("[%d] \"%s\" 0x%x ->0x%x", i, pathList[i], nd->mode, offset);

        if (IS_DIRECTORY(nd->mode)) {
            offset = nd->info;
        } else if (IS_HARDLINK(nd->mode)) {
            ret = FollowHardlinks(rm, offset, &offset);
            if (ret < 0) return ret;
            else if (ret == LINK_FOLLOWED) ROMFS_TRACE("followed hardlink");
        }
    }

   // ret = RomfsGetNodeHdr(rm, offset, nd);

    return ret;
}
