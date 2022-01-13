#include "unity_fixture.h"

#include <errno.h>

#include "romfs.h"
#include "romfs-internal.h"

extern unsigned char empty_romfs[];
extern unsigned int empty_romfs_len;
#define EMPTY_ROMFS_ROOT_OFFSET 0x20

/***************************************/
TEST_GROUP(internal);
/***************************************/

TEST_SETUP(internal)
{
}

TEST_TEAR_DOWN(internal)
{
}

TEST(internal, VolumeConfigureBadImg)
{
    uint8_t buf[10] = { 0 };
    volume_t vol;

    int ret = RomfsVolumeConfigure(buf, &vol);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);
}

TEST(internal, VolumeConfigureGoodImg)
{
    volume_t vol;

    int ret = RomfsVolumeConfigure(empty_romfs, &vol);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("empyt", vol.name, 5);
    TEST_ASSERT_EQUAL_INT(96, vol.size);
    TEST_ASSERT_EQUAL_HEX(EMPTY_ROMFS_ROOT_OFFSET, vol.rootOff);
    TEST_ASSERT_EQUAL_HEX(0xC7B9AC8D, vol.chksum);
}

TEST(internal, GetNodeHdrBadSize)
{
    romfs_t rm = { .img = empty_romfs, .size = 0, .vol.size = 96 };
    nodehdr_t node;
    int ret;

    ret = RomfsGetNodeHdr(&rm, EMPTY_ROMFS_ROOT_OFFSET, &node);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    rm.size = empty_romfs_len;
    rm.vol.size = 0;
    ret = RomfsGetNodeHdr(&rm, EMPTY_ROMFS_ROOT_OFFSET, &node);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);
}

TEST(internal, GetNodeHdrCheckData)
{
    romfs_t rm = { .img = empty_romfs, .size = empty_romfs_len, .vol.size = 96 };
    nodehdr_t node;

    int ret = RomfsGetNodeHdr(&rm, EMPTY_ROMFS_ROOT_OFFSET, &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_STRING_LEN(".", node.name, 1);
    TEST_ASSERT_EQUAL_HEX(0x40, node.next);
    TEST_ASSERT_EQUAL_HEX(ROMFS_MODE_EXEC | ROMFS_TYPE_DIRECTORY, node.mode);
    TEST_ASSERT_EQUAL_HEX(0x20, node.info);
    TEST_ASSERT_EQUAL_HEX(0, node.size);
    TEST_ASSERT_EQUAL_HEX(0xD1FFFF97, node.chksum);
    TEST_ASSERT_EQUAL_HEX(0x40, node.dataOff);
}

TEST(internal, FindEntryRoot)
{
    romfs_t rm = { .img = empty_romfs, .size = empty_romfs_len, .vol.size = 96 };
    nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm, EMPTY_ROMFS_ROOT_OFFSET, ".", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_MESSAGE(IS_TYPE(ROMFS_TYPE_DIRECTORY, node.mode), "Root should be a directory");
}

TEST_GROUP_RUNNER(internal)
{
    RUN_TEST_CASE(internal, VolumeConfigureBadImg);
    RUN_TEST_CASE(internal, VolumeConfigureGoodImg);

    RUN_TEST_CASE(internal, GetNodeHdrBadSize);
    RUN_TEST_CASE(internal, GetNodeHdrCheckData);

    RUN_TEST_CASE(internal, FindEntryRoot);
}
