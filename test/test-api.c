#include "unity_fixture.h"

#include <errno.h>

#include "romfs.h"
#include "romfs-internal.h"

extern unsigned char empty_romfs[];
extern unsigned char basic_romfs[];
extern unsigned int empty_romfs_len;
extern unsigned int basic_romfs_len;


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

    TEST_ASSERT_MESSAGE(IS_DIRECTORY(mode), "type is not directory");
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


/***************************************/
TEST_GROUP(open);
/***************************************/

TEST_SETUP(open)
{
    RomfsLoad(basic_romfs, basic_romfs_len);
}

TEST_TEAR_DOWN(open)
{
}

#define ROOT_FD 3

TEST(open, OpenAtErrorAccessingFileFromBadFD)
{
    int ret = RomfsOpenAt(0, "a", 0);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);
}

TEST(open, OpenAtBadFile)
{
    int ret = RomfsOpenAt(ROOT_FD, "Not_a_file", 0);
    TEST_ASSERT_EQUAL_INT(-ENOENT, ret);
}

TEST(open, OpenAtOpenFileAtRoot)
{
    int ret = RomfsOpenAt(ROOT_FD, "a", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);
}

TEST(open, OpenAtOpenFileAtRootAndCheckWhetherItsFile)
{
    int ret = RomfsOpenAt(ROOT_FD, "a", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);

    int mode = RomfsFdStat(ret);
    TEST_ASSERT_MESSAGE(IS_FILE(mode), "Opened file should be a regular file");
}

TEST(open, OpenAtOpenDirAtRootAndCheckWhetherItsDir)
{
    int ret = RomfsOpenAt(ROOT_FD, "/dir", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);

    int mode = RomfsFdStat(ret);
    TEST_ASSERT_MESSAGE(IS_DIRECTORY(mode), "Opened file should be a directory");
}

TEST(open, OpenAtOpenFileInDir)
{
    int ret = RomfsOpenAt(ROOT_FD, "dir/b", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);

    int mode = RomfsFdStat(ret);
    TEST_ASSERT_MESSAGE(IS_FILE(mode), "Opened file should be a regular file");
}

TEST(open, OpenAtTwoFiles)
{
    int ret = RomfsOpenAt(ROOT_FD, "/a", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);

    ret = RomfsOpenAt(ROOT_FD, "/dir/b", 0);
    TEST_ASSERT_EQUAL_INT(5, ret);
}

TEST(open, OpenAtDirAndFile)
{
    int ret = RomfsOpenAt(ROOT_FD, "dir", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);

    ret = RomfsOpenAt(ROOT_FD, "a", 0);
    TEST_ASSERT_EQUAL_INT(5, ret);

    int mode = RomfsFdStat(4);
    TEST_ASSERT_MESSAGE(IS_DIRECTORY(mode), "Opened file should be a directory");

    mode = RomfsFdStat(5);
    TEST_ASSERT_MESSAGE(IS_FILE(mode), "Opened file should be a regular file");
}

TEST(open, OpenAtPathWithHardlinks)
{
    int ret = RomfsOpenAt(ROOT_FD, "/dir/../a", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);

    int mode = RomfsFdStat(4);
    TEST_ASSERT_MESSAGE(IS_FILE(mode), "Opened file should be a regular file");

    ret = RomfsOpenAt(ROOT_FD, "../../dir/.././a", 0);
    TEST_ASSERT_EQUAL_INT(5, ret);

    mode = RomfsFdStat(5);
    TEST_ASSERT_MESSAGE(IS_FILE(mode), "Opened file should be a regular file");

    ret = RomfsOpenAt(ROOT_FD, "./dir", 0);
    TEST_ASSERT_EQUAL_INT(6, ret);

    mode = RomfsFdStat(6);
    TEST_ASSERT_MESSAGE(IS_DIRECTORY(mode), "Opened file should be a directory");
}

TEST(open, OpenAtRelativePath)
{
    int ret = RomfsOpenAt(ROOT_FD, "dir", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);

    ret = RomfsOpenAt(ret, "b", 0);
    TEST_ASSERT_EQUAL_INT(5, ret);

    int mode = RomfsFdStat(ret);
    TEST_ASSERT_MESSAGE(IS_FILE(mode), "Opened file should be a regular file");
}

TEST_GROUP_RUNNER(open)
{
    RUN_TEST_CASE(open, OpenAtErrorAccessingFileFromBadFD);
    RUN_TEST_CASE(open, OpenAtBadFile);
    RUN_TEST_CASE(open, OpenAtOpenFileAtRoot);
    RUN_TEST_CASE(open, OpenAtOpenFileAtRootAndCheckWhetherItsFile);
    RUN_TEST_CASE(open, OpenAtOpenDirAtRootAndCheckWhetherItsDir);
    RUN_TEST_CASE(open, OpenAtOpenFileInDir);
    RUN_TEST_CASE(open, OpenAtTwoFiles);
    RUN_TEST_CASE(open, OpenAtDirAndFile);
    RUN_TEST_CASE(open, OpenAtPathWithHardlinks);
    RUN_TEST_CASE(open, OpenAtRelativePath);
}

/***************************************/
TEST_GROUP(close);
/***************************************/

int openedFd;

TEST_SETUP(close)
{
    RomfsLoad(basic_romfs, basic_romfs_len);
    openedFd = RomfsOpenAt(ROOT_FD, "a", 0);
}

TEST_TEAR_DOWN(close)
{
}

TEST(close, CloseClosedFile)
{
    int ret = RomfsClose(openedFd+1);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);
}

TEST(close, CloseFile)
{
    int ret;

    ret = RomfsFdStat(openedFd);
    TEST_ASSERT(IS_FILE(ret));

    ret = RomfsClose(openedFd);
    TEST_ASSERT_EQUAL_INT(0, ret);

    ret = RomfsFdStat(openedFd);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);
}


TEST_GROUP_RUNNER(close)
{
    RUN_TEST_CASE(close, CloseFile);
    RUN_TEST_CASE(close, CloseClosedFile);
}

/***************************************/
TEST_GROUP(read);
/***************************************/

TEST_SETUP(read)
{
    RomfsLoad(basic_romfs, basic_romfs_len);
    openedFd = RomfsOpenAt(ROOT_FD, "a", 0);
}

TEST_TEAR_DOWN(read)
{
    RomfsClose(openedFd);
}

TEST(read, ReadBadFile)
{
    int r;
    char buf[10];

    r = RomfsRead(openedFd+1, buf, 10);
    TEST_ASSERT_EQUAL_INT(-EBADF, r);

    r = RomfsRead(0, buf, 10);
    TEST_ASSERT_EQUAL_INT(-EBADF, r);
}

TEST(read, ReadBadParams)
{
    int r;

    r = RomfsRead(openedFd, NULL, 10);
    TEST_ASSERT_EQUAL_INT(-EINVAL, r);
}

TEST(read, ReadFile)
{
    char buf[10];

    int r;
    r = RomfsRead(openedFd, buf, 2);
    TEST_ASSERT_EQUAL_INT(2, r);

    TEST_ASSERT_EQUAL_STRING_LEN("aa", buf, 2);

    r = RomfsRead(openedFd, buf, 2);
    TEST_ASSERT_EQUAL_INT(2, r);

    TEST_ASSERT_EQUAL_STRING_LEN("a\n", buf, 2);
}

TEST(read, ReadFileBiggerThanFileSize)
{
    char buf[10] = { 0 };

    int r;
    r = RomfsRead(openedFd, buf, 10);
    TEST_ASSERT_EQUAL_INT(4, r);

    TEST_ASSERT_EQUAL_MEMORY("aaa\n\0\0\0\0\0\0", buf, 10);
}

TEST(read, ReadFileOnceThenTryToOverflow)
{
    char buf[10] = { 0 };

    int r;
    r = RomfsRead(openedFd, buf, 2);
    TEST_ASSERT_EQUAL_INT(2, r);

    r = RomfsRead(openedFd, buf, 10);
    TEST_ASSERT_EQUAL_INT(2, r);

    TEST_ASSERT_EQUAL_MEMORY("a\n\0", buf, 3);
}

TEST(read, TryToReadFromDir)
{
    char buf[10] = { 0 };
    int fd;
    int r;

    fd = RomfsOpenAt(3, "..", 0);
    r = RomfsRead(fd, buf, 10);
    TEST_ASSERT_EQUAL_INT(-EISDIR, r);

    TEST_ASSERT_EQUAL_MEMORY("\0\0", buf, 2);
}

TEST_GROUP_RUNNER(read)
{
    RUN_TEST_CASE(read, ReadBadFile);
    RUN_TEST_CASE(read, ReadBadParams);
    RUN_TEST_CASE(read, ReadFile);
    RUN_TEST_CASE(read, ReadFileBiggerThanFileSize);
    RUN_TEST_CASE(read, ReadFileOnceThenTryToOverflow);
    RUN_TEST_CASE(read, TryToReadFromDir);
}
