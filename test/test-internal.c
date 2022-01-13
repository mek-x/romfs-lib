#include "unity_fixture.h"

#include <errno.h>

#include "romfs-internal.h"

extern unsigned char empty_romfs[];
extern unsigned int empty_romfs_len;

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
    TEST_ASSERT_EQUAL_HEX(0x20, vol.rootOff);
    TEST_ASSERT_EQUAL_HEX(0xC7B9AC8D, vol.chksum);
}

TEST_GROUP_RUNNER(internal)
{
    RUN_TEST_CASE(internal, VolumeConfigureBadImg);
    RUN_TEST_CASE(internal, VolumeConfigureGoodImg);
}
