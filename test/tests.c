
#include "romfs.h"
#include "romfs-internal.h"
#include "unity.h"

#include "empty_romfs.h"
#include "basic_romfs.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_BasicRomfsLoad(void)
{
  int ret = RomfsLoad(empty_romfs, empty_romfs_len);
  /* All of these should pass */
  TEST_ASSERT_EQUAL(0, ret);
}
