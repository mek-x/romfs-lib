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

TEST(path, ParsePathGetNextEmpty) {
    path_t buf;
    char *save;
    char *out;

    out = UtilsParsePathGetNext("", buf, &save);
    TEST_ASSERT_EQUAL_PTR(NULL, out);
}

TEST(path, ParsePathGetNextRoot) {
    path_t buf;
    char *save;
    char *out;

    out = UtilsParsePathGetNext("/", buf, &save);
    TEST_ASSERT_EQUAL_STRING(".", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_PTR(NULL, out);


    out = UtilsParsePathGetNext("///", buf, &save);
    TEST_ASSERT_EQUAL_STRING(".", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_PTR(NULL, out);


    out = UtilsParsePathGetNext("./", buf, &save);
    TEST_ASSERT_EQUAL_STRING(".", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_PTR(NULL, out);


    out = UtilsParsePathGetNext("./.", buf, &save);
    TEST_ASSERT_EQUAL_STRING(".", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_STRING(".", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_PTR(NULL, out);
}

TEST(path, ParsePathGetNextAbsoluteOneFile) {
    path_t buf;
    char *save;
    char *out;

    out = UtilsParsePathGetNext("/file", buf, &save);
    TEST_ASSERT_EQUAL_STRING(".", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_STRING("file", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_PTR(NULL, out);

    out = UtilsParsePathGetNext("/////file", buf, &save);
    TEST_ASSERT_EQUAL_STRING(".", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_STRING("file", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_PTR(NULL, out);

}

TEST(path, ParsePathGetNextRelativeTwoFiles) {
    path_t buf;
    char *save;
    char *out;

    out = UtilsParsePathGetNext("file1/file2", buf, &save);
    TEST_ASSERT_EQUAL_STRING("file1", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_STRING("file2", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_PTR(NULL, out);


    out = UtilsParsePathGetNext("a/////b//c", buf, &save);
    TEST_ASSERT_EQUAL_STRING("a", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_STRING("b", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_STRING("c", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_PTR(NULL, out);
}

TEST(path, ParsePathGetNextAbsoluteManySlashes) {
    path_t buf;
    char *save;
    char *out;

    out = UtilsParsePathGetNext("//////file1////file2///////", buf, &save);
    TEST_ASSERT_EQUAL_STRING(".", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_STRING("file1", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_STRING("file2", out);

    out = UtilsParsePathGetNext(NULL, buf, &save);
    TEST_ASSERT_EQUAL_PTR(NULL, out);
}

TEST(path, CheckPathPathTooLong)
{
    char path[MAX_PATH_LEN+1];
    int ret;

    memset(path, '/', MAX_PATH_LEN);
    path[MAX_PATH_LEN] = '\0';

    ret = UtilsCheckPath(path);
    TEST_ASSERT_EQUAL_INT(-ENAMETOOLONG, ret);
}

TEST(path, CheckPathNameTooLong)
{
    char path[MAX_NAME_LEN+1];
    int ret;

    memset(path, '0', sizeof(path));
    path[MAX_NAME_LEN] = '\0';

    ret = UtilsCheckPath(path);
    TEST_ASSERT_EQUAL_INT(-ENAMETOOLONG, ret);
}

TEST(path, CheckPathCountTheElementsInPath)
{
    char *path = "1/////22//333/4444";
    int ret;

    ret = UtilsCheckPath(path);
    TEST_ASSERT_EQUAL_INT(4, ret);
}

TEST_GROUP_RUNNER(path)
{
    RUN_TEST_CASE(path, ParsePathGetNextEmpty);
    RUN_TEST_CASE(path, ParsePathGetNextRoot);
    RUN_TEST_CASE(path, ParsePathGetNextAbsoluteOneFile);
    RUN_TEST_CASE(path, ParsePathGetNextRelativeTwoFiles);
    RUN_TEST_CASE(path, ParsePathGetNextAbsoluteManySlashes);
    RUN_TEST_CASE(path, CheckPathPathTooLong);
    RUN_TEST_CASE(path, CheckPathNameTooLong);
    RUN_TEST_CASE(path, CheckPathCountTheElementsInPath);
}
