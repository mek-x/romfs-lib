#include "unity_fixture.h"

#include "romfs.h"
#include "romfs-internal.h"

#include "empty_romfs.h"
#include "basic_romfs.h"

TEST_GROUP(load);

TEST_SETUP(load)
{
}

TEST_TEAR_DOWN(load)
{
}

TEST(load, basicRomfsLoad)
{
    int ret = RomfsLoad(empty_romfs, empty_romfs_len);
    TEST_ASSERT_EQUAL_INT(0, ret);
}
