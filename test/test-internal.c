#include "unity_fixture.h"

#include <errno.h>
#include <string.h>

#include "romfs.h"
#include "romfs-internal.h"

#define ROMFS_ROOT_OFFSET 0x20
extern unsigned char empty_romfs[];
extern unsigned int empty_romfs_len;

extern unsigned char basic_romfs[];
extern unsigned int basic_romfs_len;

/***************************************/
TEST_GROUP(volume);
/***************************************/

TEST_SETUP(volume)
{
}

TEST_TEAR_DOWN(volume)
{
}

TEST(volume, VolumeConfigureBadImg)
{
    uint8_t buf[10] = { 0 };
    volume_t vol;

    int ret = RomfsVolumeConfigure(buf, &vol);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);
}

TEST(volume, VolumeConfigureGoodImg)
{
    volume_t vol;

    int ret = RomfsVolumeConfigure(empty_romfs, &vol);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("empyt", vol.name, 5);
    TEST_ASSERT_EQUAL_INT(96, vol.size);
    TEST_ASSERT_EQUAL_HEX(ROMFS_ROOT_OFFSET, vol.rootOff);
    TEST_ASSERT_EQUAL_HEX(0xC7B9AC8D, vol.chksum);
}

TEST_GROUP_RUNNER(volume)
{
    RUN_TEST_CASE(volume, VolumeConfigureBadImg);
    RUN_TEST_CASE(volume, VolumeConfigureGoodImg);
}

/***************************************/
TEST_GROUP(nodes);
/***************************************/

romfs_t rm;

TEST_SETUP(nodes)
{
    rm.img = empty_romfs;
    rm.size = empty_romfs_len;
    rm.vol.size = 96;
}

TEST_TEAR_DOWN(nodes)
{
    memset(&rm, 0, sizeof(rm));
}

TEST(nodes, GetNodeHdrBadSize)
{
    nodehdr_t node;
    int ret;

    rm.size = 0;
    ret = RomfsGetNodeHdr(&rm, ROMFS_ROOT_OFFSET, &node);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    rm.size = empty_romfs_len;
    rm.vol.size = 0;
    ret = RomfsGetNodeHdr(&rm, ROMFS_ROOT_OFFSET, &node);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);
}

TEST(nodes, GetNodeHdrCheckData)
{
    nodehdr_t node;

    int ret = RomfsGetNodeHdr(&rm, ROMFS_ROOT_OFFSET, &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_STRING_LEN(".", node.name, 1);
    TEST_ASSERT_EQUAL_HEX(0x40, node.next);
    TEST_ASSERT_EQUAL_HEX(ROMFS_MODE_EXEC | ROMFS_TYPE_DIRECTORY, node.mode);
    TEST_ASSERT_EQUAL_HEX(0x20, node.info);
    TEST_ASSERT_EQUAL_HEX(0, node.size);
    TEST_ASSERT_EQUAL_HEX(0xD1FFFF97, node.chksum);
    TEST_ASSERT_EQUAL_HEX(0x40, node.dataOff);
}

TEST(nodes, FindEntryRoot)
{
    nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm, ROMFS_ROOT_OFFSET, ".", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_MESSAGE(IS_TYPE(ROMFS_TYPE_DIRECTORY, node.mode), "Root should be a directory");
}

TEST(nodes, FindEntryFollowHardlink)
{
    nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm, 0x40, ".", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_MESSAGE(IS_TYPE(ROMFS_TYPE_DIRECTORY, node.mode), "Root should be a directory");
}

TEST_GROUP_RUNNER(nodes)
{
    RUN_TEST_CASE(nodes, GetNodeHdrBadSize);
    RUN_TEST_CASE(nodes, GetNodeHdrCheckData);

    RUN_TEST_CASE(nodes, FindEntryRoot);
    RUN_TEST_CASE(nodes, FindEntryFollowHardlink);
}

/***************************************/
TEST_GROUP(path);
/***************************************/

romfs_t rm_basic;

TEST_SETUP(path)
{
    rm_basic.img = basic_romfs;
    rm_basic.size = basic_romfs_len;
    rm_basic.vol.size = 288;
}

TEST_TEAR_DOWN(path)
{
    memset(&rm_basic, 0, sizeof(rm));
}

TEST(path, FindEntryFileNotFound)
{
     nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm_basic, ROMFS_ROOT_OFFSET, "not_a_file", &node);
    TEST_ASSERT_EQUAL_INT(-ENOENT, ret);
}

TEST(path, SearchDirNotFound)
{
    int ret;
    uint32_t offset = ROMFS_ROOT_OFFSET;

    ret = RomfsSearchDir(&rm_basic, "not_a_file", &offset);
    TEST_ASSERT_EQUAL_INT(-ENOENT, ret);
}

TEST(path, SearchDirFoundFile)
{
    int ret;
    uint32_t offset = ROMFS_ROOT_OFFSET;

    ret = RomfsSearchDir(&rm_basic, "a", &offset);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(0xf0, offset);
}

TEST(path, ParsePathEmpty)
{
    char *path = "";
    filename_t parsed[1];
    int ret;
    
    ret = RomfsParsePath(path, parsed, 1);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_STRING("", parsed[0]);
}

TEST(path, ParsePathRoot)
{
    char *path = "/";
    filename_t parsed[1];
    int ret;
    
    ret = RomfsParsePath(path, parsed, 1);
    TEST_ASSERT_EQUAL_INT(1, ret);

    TEST_ASSERT_EQUAL_STRING("/", parsed[0]);
}

TEST(path, ParsePathTooLong)
{
    char path[300];
    filename_t parsed[1];
    int ret;

    memset(path, '/', 299);
    path[299] = '\0';

    ret = RomfsParsePath(path, parsed, 1);
    TEST_ASSERT_EQUAL_INT(-ENAMETOOLONG, ret);
}

TEST(path, ParsePathArrayLenHasToBeBiggerThan0)
{
    int ret;
    char *path = "";
    filename_t parsed[1];

    ret = RomfsParsePath(path, parsed, 0);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);
}

TEST(path, ParsePathFileInRootDir)
{
    char *path = "////file";
    filename_t parsed[2];
    int ret;

    ret = RomfsParsePath(path, parsed, 2);
    TEST_ASSERT_EQUAL_INT(2, ret);

    TEST_ASSERT_EQUAL_STRING("/", parsed[0]);
    TEST_ASSERT_EQUAL_STRING("file", parsed[1]);
}

TEST(path, ParsePathFileInDirRelative)
{
    char *path = "a/////b//c";
    filename_t parsed[3];
    int ret;

    ret = RomfsParsePath(path, parsed, 3);
    TEST_ASSERT_EQUAL_INT(3, ret);

    TEST_ASSERT_EQUAL_STRING("a", parsed[0]);
    TEST_ASSERT_EQUAL_STRING("b", parsed[1]);
    TEST_ASSERT_EQUAL_STRING("c", parsed[2]);
}

    
TEST_GROUP_RUNNER(path)
{
    RUN_TEST_CASE(path, FindEntryFileNotFound);
    RUN_TEST_CASE(path, SearchDirNotFound);
    RUN_TEST_CASE(path, SearchDirFoundFile);
    RUN_TEST_CASE(path, ParsePathEmpty);
    RUN_TEST_CASE(path, ParsePathRoot);
    RUN_TEST_CASE(path, ParsePathTooLong);
    RUN_TEST_CASE(path, ParsePathArrayLenHasToBeBiggerThan0);
    RUN_TEST_CASE(path, ParsePathFileInRootDir);
    RUN_TEST_CASE(path, ParsePathFileInDirRelative);
}
