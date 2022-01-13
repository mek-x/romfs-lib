#include "unity_fixture.h"

#include "romfs.h"
#include "romfs-internal.h"

extern unsigned char empty_romfs[];
extern unsigned int empty_romfs_len;

/***************************************/
TEST_GROUP(internal);
/***************************************/

TEST_SETUP(internal)
{
    RomfsLoad(empty_romfs, empty_romfs_len);
}

TEST_TEAR_DOWN(internal)
{
}

TEST(internal, SomeTest)
{
    TEST_ASSERT(0);
}

TEST_GROUP_RUNNER(internal)
{
    RUN_TEST_CASE(internal, SomeTest);
}
