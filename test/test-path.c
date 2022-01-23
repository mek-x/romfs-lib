#include "common_test_defines.h"

#include <path_utils.h>

/***************************************/
TEST_GROUP(path);
/***************************************/

TEST_SETUP(path)
{
}

TEST_TEAR_DOWN(path)
{
}

TEST(path, ParsePathEmpty)
{
    char *path = "";
    filename_t parsed[1];
    int ret;

    ret = UtilsParsePath(path, parsed, 1);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_STRING("", parsed[0]);
}

TEST(path, ParsePathRoot)
{
    filename_t parsed[2];
    int ret;

    ret = UtilsParsePath("/", parsed, 2);
    TEST_ASSERT_EQUAL_INT(1, ret);

    TEST_ASSERT_EQUAL_STRING(".", parsed[0]);

    ret = UtilsParsePath("///", parsed, 2);
    TEST_ASSERT_EQUAL_INT(1, ret);

    TEST_ASSERT_EQUAL_STRING(".", parsed[0]);

    ret = UtilsParsePath("./", parsed, 2);
    TEST_ASSERT_EQUAL_INT(1, ret);

    TEST_ASSERT_EQUAL_STRING(".", parsed[0]);

    ret = UtilsParsePath("./.", parsed, 2);
    TEST_ASSERT_EQUAL_INT(2, ret);

    TEST_ASSERT_EQUAL_STRING(".", parsed[0]);
    TEST_ASSERT_EQUAL_STRING(".", parsed[1]);
}

TEST(path, ParsePathPathTooLong)
{
    char path[400];
    filename_t parsed[1];
    int ret;

    memset(path, '/', 399);
    path[399] = '\0';

    ret = UtilsParsePath(path, parsed, 1);
    TEST_ASSERT_EQUAL_INT(-ENAMETOOLONG, ret);
}

TEST(path, ParsePathNameTooLong)
{
    char path[] = "0123456789012345678901234567890";
    filename_t parsed[1];
    int ret;

    ret = UtilsParsePath(path, parsed, 1);
    TEST_ASSERT_EQUAL_INT(-ENAMETOOLONG, ret);
}

TEST(path, ParsePathArrayLenHasToBeBiggerThan0)
{
    int ret;
    char *path = "";
    filename_t parsed[1];

    ret = UtilsParsePath(path, parsed, 0);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);
}

TEST(path, ParsePathFileInRootDir)
{
    char *path = "////file";
    filename_t parsed[2];
    int ret;

    ret = UtilsParsePath(path, parsed, 2);
    TEST_ASSERT_EQUAL_INT(2, ret);

    TEST_ASSERT_EQUAL_STRING(".", parsed[0]);
    TEST_ASSERT_EQUAL_STRING("file", parsed[1]);
}

TEST(path, ParsePathFileInDirRelative)
{
    char *path = "a/////b//c";
    filename_t parsed[3];
    int ret;

    ret = UtilsParsePath(path, parsed, 3);
    TEST_ASSERT_EQUAL_INT(3, ret);

    TEST_ASSERT_EQUAL_STRING("a", parsed[0]);
    TEST_ASSERT_EQUAL_STRING("b", parsed[1]);
    TEST_ASSERT_EQUAL_STRING("c", parsed[2]);
}

TEST(path, ParsePathCountTheElementsInPath)
{
    char *path = "1/////22//333/4444";
    int ret;

    ret = UtilsParsePath(path, NULL, 0);
    TEST_ASSERT_EQUAL_INT(4, ret);
}

TEST_GROUP_RUNNER(path)
{
    RUN_TEST_CASE(path, ParsePathEmpty);
    RUN_TEST_CASE(path, ParsePathRoot);
    RUN_TEST_CASE(path, ParsePathPathTooLong);
    RUN_TEST_CASE(path, ParsePathNameTooLong);
    RUN_TEST_CASE(path, ParsePathArrayLenHasToBeBiggerThan0);
    RUN_TEST_CASE(path, ParsePathFileInRootDir);
    RUN_TEST_CASE(path, ParsePathFileInDirRelative);
    RUN_TEST_CASE(path, ParsePathCountTheElementsInPath);
}
