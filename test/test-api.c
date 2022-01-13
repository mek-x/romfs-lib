#include "unity_fixture.h"

#include <errno.h>

#include "romfs.h"
#include "romfs-internal.h"

extern unsigned char empty_romfs[];
extern unsigned int empty_romfs_len;


/***************************************/
TEST_GROUP(load);
/***************************************/

TEST_SETUP(load)
{
}

TEST_TEAR_DOWN(load)
{
}

TEST(load, LoadEmptyRomfsImage)
{
    int ret = RomfsLoad(empty_romfs, empty_romfs_len);
    TEST_ASSERT_EQUAL_INT(0, ret);
}

TEST(load, LoadBadImage)
{
    uint8_t badImage[] = { 0x00, 0x00, 0x00, 0x00 };

    int ret = RomfsLoad(badImage, sizeof(badImage));
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);
}

TEST_GROUP_RUNNER(load)
{
    RUN_TEST_CASE(load, LoadEmptyRomfsImage);
    RUN_TEST_CASE(load, LoadBadImage);
}


/***************************************/
TEST_GROUP(stat);
/***************************************/

TEST_SETUP(stat)
{
    RomfsLoad(empty_romfs, empty_romfs_len);
}

TEST_TEAR_DOWN(stat)
{
}

TEST(stat, StatCheckRootFd)
{
    int mode = RomfsFdStat(3);

    TEST_ASSERT_MESSAGE(IS_TYPE(ROMFS_TYPE_DIRECTORY, mode), "type is not directory");
}

TEST(stat, StatCheckBadFd)
{
    int mode = RomfsFdStat(0);

    TEST_ASSERT_EQUAL_INT_MESSAGE(-EBADF, mode, "expecting EBADF error");
}

TEST_GROUP_RUNNER(stat)
{
    RUN_TEST_CASE(stat, StatCheckRootFd);
    RUN_TEST_CASE(stat, StatCheckBadFd);
}
