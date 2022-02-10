#include "common_test_defines.h"

/* GLOBALS */
int openedFd;
romfs_dirent_t dirBuf[10];
size_t dirBufUsed;

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

    int mode = RomfsFdStat(ret, NULL);
    TEST_ASSERT_MESSAGE(IS_FILE(mode), "Opened file should be a regular file");
}

TEST(open, OpenAtOpenDirAtRootAndCheckWhetherItsDir)
{
    int ret = RomfsOpenAt(ROOT_FD, "/dir", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);

    int mode = RomfsFdStat(ret, NULL);
    TEST_ASSERT_MESSAGE(IS_DIRECTORY(mode), "Opened file should be a directory");
}

TEST(open, OpenAtOpenFileInDir)
{
    int ret = RomfsOpenAt(ROOT_FD, "dir/b", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);

    int mode = RomfsFdStat(ret, NULL);
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

    int mode = RomfsFdStat(4, NULL);
    TEST_ASSERT_MESSAGE(IS_DIRECTORY(mode), "Opened file should be a directory");

    mode = RomfsFdStat(5, NULL);
    TEST_ASSERT_MESSAGE(IS_FILE(mode), "Opened file should be a regular file");
}

TEST(open, OpenAtPathWithHardlinks)
{
    int ret = RomfsOpenAt(ROOT_FD, "/dir/../a", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);

    int mode = RomfsFdStat(4, NULL);
    TEST_ASSERT_MESSAGE(IS_FILE(mode), "Opened file should be a regular file");

    ret = RomfsOpenAt(ROOT_FD, "../../dir/.././a", 0);
    TEST_ASSERT_EQUAL_INT(5, ret);

    mode = RomfsFdStat(5, NULL);
    TEST_ASSERT_MESSAGE(IS_FILE(mode), "Opened file should be a regular file");

    ret = RomfsOpenAt(ROOT_FD, "./dir", 0);
    TEST_ASSERT_EQUAL_INT(6, ret);

    mode = RomfsFdStat(6, NULL);
    TEST_ASSERT_MESSAGE(IS_DIRECTORY(mode), "Opened file should be a directory");
}

TEST(open, OpenAtRelativePath)
{
    int ret = RomfsOpenAt(ROOT_FD, "dir", 0);
    TEST_ASSERT_EQUAL_INT(4, ret);

    ret = RomfsOpenAt(ret, "b", 0);
    TEST_ASSERT_EQUAL_INT(5, ret);

    int mode = RomfsFdStat(ret, NULL);
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

    ret = RomfsFdStat(openedFd, NULL);
    TEST_ASSERT(IS_FILE(ret));

    ret = RomfsClose(openedFd);
    TEST_ASSERT_EQUAL_INT(0, ret);

    ret = RomfsFdStat(openedFd, NULL);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);
}


TEST_GROUP_RUNNER(close)
{
    RUN_TEST_CASE(close, CloseFile);
    RUN_TEST_CASE(close, CloseClosedFile);
}

/***************************************/
TEST_GROUP(stat);
/***************************************/

TEST_SETUP(stat)
{
    RomfsLoad(basic_romfs, basic_romfs_len);
}

TEST_TEAR_DOWN(stat)
{
}

TEST(stat, StatCheckBadFd)
{
    int mode = RomfsFdStat(0, NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(-EBADF, mode, "expecting EBADF error");
}

TEST(stat, StatAtNoFile)
{
    int mode = RomfsFdStatAt(ROOT_FD, "x", NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(-ENOENT, mode, "expecting ENOENT error");
}

TEST(stat, StatCheckRootFd)
{
    int mode = RomfsFdStat(ROOT_FD, NULL);

    TEST_ASSERT_MESSAGE(IS_DIRECTORY(mode), "type is not directory");
}

TEST(stat, StatCheckMoreStats)
{
    romfs_stat_t stat;
    int fd = RomfsOpenAt(ROOT_FD, "a", 0);

    int mode = RomfsFdStat(fd, &stat);

    TEST_ASSERT_MESSAGE(IS_FILE(mode), "type is not file");
    TEST_ASSERT_MESSAGE(IS_FILE(stat.mode), "type is not file");
    TEST_ASSERT_EQUAL_INT(4, stat.size);
    TEST_ASSERT_EQUAL_HEX(0x9EFFFFFA, stat.chksum);
    TEST_ASSERT_EQUAL_HEX(A_FILE_OFFSET, stat.ino);
}

TEST(stat, StatAtCheckMoreStats)
{
    romfs_stat_t stat;
    int mode = RomfsFdStatAt(ROOT_FD, "dir/b", &stat);

    TEST_ASSERT_MESSAGE(IS_FILE(mode), "type is not file");
    TEST_ASSERT_MESSAGE(IS_FILE(stat.mode), "type is not file");
    TEST_ASSERT_EQUAL_INT(4, stat.size);
    TEST_ASSERT_EQUAL_HEX(0x9DFFFF2A, stat.chksum);
    TEST_ASSERT_EQUAL_HEX(B_FILE_OFFSET, stat.ino);
}

TEST_GROUP_RUNNER(stat)
{
    RUN_TEST_CASE(stat, StatCheckBadFd);
    RUN_TEST_CASE(stat, StatAtNoFile);
    RUN_TEST_CASE(stat, StatCheckRootFd);
    RUN_TEST_CASE(stat, StatCheckMoreStats);
    RUN_TEST_CASE(stat, StatAtCheckMoreStats);
}

/***************************************/
TEST_GROUP(readFile);
/***************************************/

TEST_SETUP(readFile)
{
    RomfsLoad(basic_romfs, basic_romfs_len);
    openedFd = RomfsOpenAt(ROOT_FD, "a", 0);
}

TEST_TEAR_DOWN(readFile)
{
    RomfsClose(openedFd);
}

TEST(readFile, ReadBadFile)
{
    int r;
    char buf[10];

    r = RomfsRead(openedFd+1, buf, 10);
    TEST_ASSERT_EQUAL_INT(-EBADF, r);

    r = RomfsRead(0, buf, 10);
    TEST_ASSERT_EQUAL_INT(-EBADF, r);
}

TEST(readFile, ReadBadParams)
{
    int r;

    r = RomfsRead(openedFd, NULL, 10);
    TEST_ASSERT_EQUAL_INT(-EINVAL, r);
}

TEST(readFile, ReadFile)
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

TEST(readFile, ReadFileBiggerThanFileSize)
{
    char buf[10] = { 0 };

    int r;
    r = RomfsRead(openedFd, buf, 10);
    TEST_ASSERT_EQUAL_INT(4, r);

    TEST_ASSERT_EQUAL_MEMORY("aaa\n\0\0\0\0\0\0", buf, 10);
}

TEST(readFile, ReadFileOnceThenTryToOverflow)
{
    char buf[10] = { 0 };

    int r;
    r = RomfsRead(openedFd, buf, 2);
    TEST_ASSERT_EQUAL_INT(2, r);

    r = RomfsRead(openedFd, buf, 10);
    TEST_ASSERT_EQUAL_INT(2, r);

    TEST_ASSERT_EQUAL_MEMORY("a\n\0", buf, 3);
}

TEST(readFile, TryToReadFromDir)
{
    char buf[10] = { 0 };
    int fd;
    int r;

    fd = RomfsOpenAt(3, "..", 0);
    r = RomfsRead(fd, buf, 10);
    TEST_ASSERT_EQUAL_INT(-EISDIR, r);

    TEST_ASSERT_EQUAL_MEMORY("\0\0", buf, 2);
}

TEST_GROUP_RUNNER(readFile)
{
    RUN_TEST_CASE(readFile, ReadBadFile);
    RUN_TEST_CASE(readFile, ReadBadParams);
    RUN_TEST_CASE(readFile, ReadFile);
    RUN_TEST_CASE(readFile, ReadFileBiggerThanFileSize);
    RUN_TEST_CASE(readFile, ReadFileOnceThenTryToOverflow);
    RUN_TEST_CASE(readFile, TryToReadFromDir);
}

/***************************************/
TEST_GROUP(seek);
/***************************************/

TEST_SETUP(seek)
{
    RomfsLoad(basic_romfs, basic_romfs_len);
    openedFd = RomfsOpenAt(ROOT_FD, "a", 0);
}

TEST_TEAR_DOWN(seek)
{
    RomfsClose(openedFd);
}

TEST(seek, SeekParamErrors) {
    int ret;

    ret = RomfsSeek(openedFd, 0, 3);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsSeek(ROOT_FD, 0, ROMFS_SEEK_SET);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);

    ret = RomfsSeek(10, 0, ROMFS_SEEK_SET);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);

    ret = RomfsSeek(openedFd, 10, 3);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsSeek(openedFd, -10, 3);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);
}

TEST(seek, SeekSet) {
    int ret;
    char buf[10];

    ret = RomfsSeek(openedFd, 5, ROMFS_SEEK_SET);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsSeek(openedFd, -1, ROMFS_SEEK_SET);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsSeek(openedFd, 1, ROMFS_SEEK_SET);
    TEST_ASSERT_EQUAL_INT(0, ret);

    ret = RomfsRead(openedFd, buf, 10);
    TEST_ASSERT_EQUAL_INT(3, ret);

    TEST_ASSERT_EQUAL_STRING_LEN("aa\n", buf, 3);
}

TEST(seek, SeekCur) {
    int ret;
    char buf[10];

    ret = RomfsRead(openedFd, buf, 1);
    TEST_ASSERT_EQUAL_INT(1, ret);

    ret = RomfsSeek(openedFd, -2, ROMFS_SEEK_CUR);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsSeek(openedFd, 4, ROMFS_SEEK_CUR);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsSeek(openedFd, -1, ROMFS_SEEK_CUR);
    TEST_ASSERT_EQUAL_INT(0, ret);

    ret = RomfsRead(openedFd, buf, 10);
    TEST_ASSERT_EQUAL_INT(4, ret);

    TEST_ASSERT_EQUAL_STRING_LEN("aaa\n", buf, 4);
}

TEST(seek, SeekEnd) {
    int ret;
    char buf[10];

    ret = RomfsSeek(openedFd, 1, ROMFS_SEEK_END);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsSeek(openedFd, -5, ROMFS_SEEK_END);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsSeek(openedFd, -1, ROMFS_SEEK_END);
    TEST_ASSERT_EQUAL_INT(0, ret);

    ret = RomfsRead(openedFd, buf, 10);
    TEST_ASSERT_EQUAL_INT(1, ret);

    TEST_ASSERT_EQUAL_STRING_LEN("\n", buf, 1);
}

TEST_GROUP_RUNNER(seek)
{
    RUN_TEST_CASE(seek, SeekParamErrors);
    RUN_TEST_CASE(seek, SeekSet);
    RUN_TEST_CASE(seek, SeekCur);
    RUN_TEST_CASE(seek, SeekEnd);
}

/***************************************/
TEST_GROUP(tell);
/***************************************/

TEST_SETUP(tell)
{
    RomfsLoad(basic_romfs, basic_romfs_len);
    openedFd = RomfsOpenAt(ROOT_FD, "a", 0);
}

TEST_TEAR_DOWN(tell)
{
    RomfsClose(openedFd);
}

TEST(tell, TellParamErrors)
{
    int ret = 0;
    long off;

    ret = RomfsTell(ROOT_FD, &off);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);

    ret = RomfsTell(openedFd, NULL);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsTell(-1, &off);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);

    ret = RomfsTell(openedFd+1, &off);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);
}

TEST(tell, TellOk)
{
    int ret = 0;
    long off = 0;

    ret = RomfsSeek(openedFd, 1, ROMFS_SEEK_SET);
    TEST_ASSERT_EQUAL_INT(0, ret);

    ret = RomfsTell(openedFd, &off);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_INT(1, off);
}

TEST(tell, TellEnd)
{
    int ret = 0;
    long off = 0;

    ret = RomfsSeek(openedFd, 0, ROMFS_SEEK_END);
    TEST_ASSERT_EQUAL_INT(0, ret);

    ret = RomfsTell(openedFd, &off);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_INT(4, off);
}

TEST_GROUP_RUNNER(tell)
{
    RUN_TEST_CASE(tell, TellParamErrors);
    RUN_TEST_CASE(tell, TellOk);
    RUN_TEST_CASE(tell, TellEnd);
}

/***************************************/
TEST_GROUP(readDir);
/***************************************/

TEST_SETUP(readDir)
{
    RomfsLoad(basic_romfs, basic_romfs_len);
    memset(dirBuf, 0, sizeof(dirBuf));
    dirBufUsed = 0;
}

TEST_TEAR_DOWN(readDir)
{
}

TEST(readDir, ReadDirInvalidParams)
{
    int ret;
    uint32_t cookie = 32;

    ret = RomfsReadDir(ROOT_FD, NULL, 10, &cookie, &dirBufUsed);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsReadDir(ROOT_FD, dirBuf, 10, &cookie, NULL);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsReadDir(ROOT_FD, NULL, 10, &cookie, NULL);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsReadDir(ROOT_FD, dirBuf, 10, NULL, &dirBufUsed);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);
}

TEST(readDir, ReadDirBadFileDescriptor)
{
    int ret;
    uint32_t cookie = 32;

    ret = RomfsReadDir(0, dirBuf, 10, &cookie, &dirBufUsed);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);

    ret = RomfsReadDir(9999, dirBuf, 10, &cookie, &dirBufUsed);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);

    ret = RomfsReadDir(8, dirBuf, 10, &cookie, &dirBufUsed);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);
}

TEST(readDir, ReadDirNotDirectory)
{
    int ret;
    uint32_t cookie = 32;

    ret = RomfsOpenAt(ROOT_FD, "a", 0);

    ret = RomfsReadDir(ret, dirBuf, 10, &cookie, &dirBufUsed);
    TEST_ASSERT_EQUAL_INT(-ENOTDIR, ret);
}

TEST(readDir, ReadDirRootDir)
{
    int ret;
    uint32_t cookie = 0;

    ret = RomfsReadDir(ROOT_FD, dirBuf, 10, &cookie, &dirBufUsed);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_INT(4, dirBufUsed);

    TEST_ASSERT_EQUAL_STRING_LEN(".", dirBuf[0].name, 1);
    TEST_ASSERT_EQUAL_INT(1, dirBuf[0].nameLen);
    TEST_ASSERT_EQUAL_HEX(ROOT_OFFSET, dirBuf[0].inode);
    TEST_ASSERT_EQUAL_HEX(SECOND_ENTRY_ROOT_OFF, dirBuf[0].next);
    TEST_ASSERT(IS_DIRECTORY(dirBuf[0].type));

    TEST_ASSERT_EQUAL_STRING_LEN("..", dirBuf[1].name, 2);
    TEST_ASSERT_EQUAL_INT(2, dirBuf[1].nameLen);
    TEST_ASSERT_EQUAL_HEX(SECOND_ENTRY_ROOT_OFF, dirBuf[1].inode);
    TEST_ASSERT_EQUAL_HEX(DIR_OFFSET, dirBuf[1].next);
    TEST_ASSERT(IS_HARDLINK(dirBuf[1].type));

    TEST_ASSERT_EQUAL_STRING_LEN("dir", dirBuf[2].name, 3);
    TEST_ASSERT_EQUAL_INT(3, dirBuf[2].nameLen);
    TEST_ASSERT_EQUAL_HEX(DIR_OFFSET, dirBuf[2].inode);
    TEST_ASSERT_EQUAL_HEX(A_FILE_OFFSET, dirBuf[2].next);
    TEST_ASSERT(IS_DIRECTORY(dirBuf[2].type));

    TEST_ASSERT_EQUAL_STRING_LEN("a", dirBuf[3].name, 1);
    TEST_ASSERT_EQUAL_INT(1, dirBuf[3].nameLen);
    TEST_ASSERT_EQUAL_HEX(A_FILE_OFFSET, dirBuf[3].inode);
    TEST_ASSERT_EQUAL_HEX(0x0, dirBuf[3].next);
    TEST_ASSERT(IS_FILE(dirBuf[3].type));
}

TEST(readDir, ReadDirInDir)
{
    int ret;
    uint32_t cookie = 0;

    ret = RomfsOpenAt(ROOT_FD, "dir", 0);

    ret = RomfsReadDir(ret, dirBuf, 10, &cookie, &dirBufUsed);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_INT(3, dirBufUsed);

    TEST_ASSERT_EQUAL_STRING_LEN("..", dirBuf[0].name, 2);
    TEST_ASSERT_EQUAL_INT(2, dirBuf[0].nameLen);
    TEST_ASSERT_EQUAL_HEX(FIRST_ENTRY_IN_DIR, dirBuf[0].inode);
    TEST_ASSERT_EQUAL_HEX(B_FILE_OFFSET, dirBuf[0].next);
    TEST_ASSERT(IS_HARDLINK(dirBuf[0].type));

    TEST_ASSERT_EQUAL_STRING_LEN("b", dirBuf[1].name, 1);
    TEST_ASSERT_EQUAL_INT(1, dirBuf[1].nameLen);
    TEST_ASSERT_EQUAL_HEX(B_FILE_OFFSET, dirBuf[1].inode);
    TEST_ASSERT_EQUAL_HEX(DIR_IN_DIR_OFFSET, dirBuf[1].next);
    TEST_ASSERT(IS_FILE(dirBuf[1].type));

    TEST_ASSERT_EQUAL_STRING_LEN(".", dirBuf[2].name, 1);
    TEST_ASSERT_EQUAL_INT(1, dirBuf[2].nameLen);
    TEST_ASSERT_EQUAL_HEX(DIR_IN_DIR_OFFSET, dirBuf[2].inode);
    TEST_ASSERT_EQUAL_HEX(0x0, dirBuf[2].next);
    TEST_ASSERT(IS_HARDLINK(dirBuf[2].type));
}

TEST(readDir, ReadDirUsingCookie)
{
    int ret;
    uint32_t cookie = 0;

    ret = RomfsReadDir(ROOT_FD, dirBuf, 2, &cookie, &dirBufUsed);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_INT(2, dirBufUsed);
    TEST_ASSERT_EQUAL_HEX(DIR_OFFSET, cookie);

    TEST_ASSERT_EQUAL_STRING_LEN(".", dirBuf[0].name, 1);
    TEST_ASSERT_EQUAL_STRING_LEN("..", dirBuf[1].name, 2);

    ret = RomfsReadDir(ROOT_FD, dirBuf, 2, &cookie, &dirBufUsed);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_INT(2, dirBufUsed);
    TEST_ASSERT_EQUAL_HEX(ROMFS_COOKIE_LAST, cookie);

    TEST_ASSERT_EQUAL_STRING_LEN("dir", dirBuf[0].name, 3);
    TEST_ASSERT_EQUAL_STRING_LEN("a", dirBuf[1].name, 1);
}

TEST_GROUP_RUNNER(readDir)
{
    RUN_TEST_CASE(readDir, ReadDirInvalidParams);
    RUN_TEST_CASE(readDir, ReadDirBadFileDescriptor);
    RUN_TEST_CASE(readDir, ReadDirNotDirectory);
    RUN_TEST_CASE(readDir, ReadDirRootDir);
    RUN_TEST_CASE(readDir, ReadDirInDir);
    RUN_TEST_CASE(readDir, ReadDirUsingCookie);
}

/***************************************/
TEST_GROUP(mapFile);
/***************************************/

TEST_SETUP(mapFile)
{
    RomfsLoad(basic_romfs, basic_romfs_len);
    openedFd = RomfsOpenAt(ROOT_FD, "a", 0);
}

TEST_TEAR_DOWN(mapFile)
{
    RomfsClose(openedFd);
}

TEST(mapFile, MapError)
{
    uint8_t *addr;
    size_t len;
    int ret;

    ret = RomfsMapFile((void **)&addr, &len, 0, 0);
    TEST_ASSERT_EQUAL_INT(-EBADF, ret);

    ret = RomfsMapFile((void **)&addr, &len, openedFd, 100);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    ret = RomfsMapFile((void **)&addr, &len, ROOT_FD, 0);
    TEST_ASSERT_EQUAL_INT(-EACCES, ret);
}

TEST(mapFile, BasicMap)
{
    uint8_t *addr;
    size_t len;

    int ret = RomfsMapFile((void **)&addr, &len, openedFd, 0);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_INT(4, len);
    TEST_ASSERT_EQUAL_MEMORY("aaa\n", addr, len);
}

TEST(mapFile, MapWithOffset)
{
    uint8_t *addr;
    size_t len;

    int ret = RomfsMapFile((void **)&addr, &len, openedFd, 2);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_INT(2, len);
    TEST_ASSERT_EQUAL_MEMORY("a\n", addr, len);
}

TEST_GROUP_RUNNER(mapFile)
{
    RUN_TEST_CASE(mapFile, MapError);
    RUN_TEST_CASE(mapFile, BasicMap);
    RUN_TEST_CASE(mapFile, MapWithOffset);
}
