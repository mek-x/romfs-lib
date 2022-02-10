#include "common_test_defines.h"

#include <path_utils.h>

/***************************************/
TEST_GROUP(multi);
/***************************************/

TEST_SETUP(multi)
{
}

TEST_TEAR_DOWN(multi)
{
}

TEST(multi, LoadTwoRomfs)
{
    int ret;
    romfs_t r1, r2;

    ret = RomfsLoad(basic_romfs, basic_romfs_len, &r1);
    ret = RomfsLoad(advanced_romfs, advanced_romfs_len, &r2);

    ret = RomfsOpenRoot(r1, "b", 0);
    TEST_ASSERT_EQUAL_INT(-ENOENT, ret);

    ret = RomfsOpenRoot(r2, "b", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);
    RomfsClose(r2, ret);

    RomfsUnload(&r1);

    ret = RomfsOpenRoot(r2, "b", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);
    RomfsClose(r2, ret);

    RomfsUnload(&r2);
}


TEST_GROUP_RUNNER(multi)
{
    RUN_TEST_CASE(multi, LoadTwoRomfs);
}
