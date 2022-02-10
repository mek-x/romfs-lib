#include "common_test_defines.h"

/* GLOBALS */

struct romfs_t rm_basic;
struct romfs_t rm_empty;

/***************************************/
TEST_GROUP(volume);
/***************************************/

TEST_SETUP(volume)
{
}

TEST_TEAR_DOWN(volume)
{
}

TEST(volume, VolumeConfigureBadImg)
{
    uint8_t buf[10] = { 0 };
    volume_t vol;

    int ret = RomfsVolumeConfigure(buf, &vol);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);
}

TEST(volume, VolumeConfigureGoodImg)
{
    volume_t vol;

    int ret = RomfsVolumeConfigure(empty_romfs, &vol);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("empyt", vol.name, 5);
    TEST_ASSERT_EQUAL_INT(96, vol.size);
    TEST_ASSERT_EQUAL_HEX(ROOT_OFFSET, vol.rootOff);
    TEST_ASSERT_EQUAL_HEX(0xC7B9AC8D, vol.chksum);
}

TEST_GROUP_RUNNER(volume)
{
    RUN_TEST_CASE(volume, VolumeConfigureBadImg);
    RUN_TEST_CASE(volume, VolumeConfigureGoodImg);
}

/***************************************/
TEST_GROUP(nodes);
/***************************************/

TEST_SETUP(nodes)
{
    rm_empty.img = empty_romfs;
    rm_empty.size = empty_romfs_len;
    rm_empty.vol.size = 96;
    rm_empty.vol.rootOff = ROOT_OFFSET;
}

TEST_TEAR_DOWN(nodes)
{
    memset(&rm_empty, 0, sizeof(rm_empty));
}

TEST(nodes, GetNodeHdrBadSize)
{
    nodehdr_t node;
    int ret;

    rm_empty.size = 0;
    ret = RomfsGetNodeHdr(&rm_empty, ROOT_OFFSET, &node);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);

    rm_empty.size = empty_romfs_len;
    rm_empty.vol.size = 0;
    ret = RomfsGetNodeHdr(&rm_empty, ROOT_OFFSET, &node);
    TEST_ASSERT_EQUAL_INT(-EINVAL, ret);
}

TEST(nodes, GetNodeHdrCheckData)
{
    nodehdr_t node;

    int ret = RomfsGetNodeHdr(&rm_empty, ROOT_OFFSET, &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_STRING_LEN(".", node.name, 1);
    TEST_ASSERT_EQUAL_HEX(0x40, node.next);
    TEST_ASSERT_EQUAL_HEX(ROMFS_MODE_EXEC | ROMFS_TYPE_DIRECTORY, node.mode);
    TEST_ASSERT_EQUAL_HEX(0x20, node.info);
    TEST_ASSERT_EQUAL_HEX(0, node.size);
    TEST_ASSERT_EQUAL_HEX(0xD1FFFF97, node.chksum);
    TEST_ASSERT_EQUAL_HEX(0x40, node.dataOff);
}

TEST(nodes, FindEntryRoot)
{
    nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm_empty, ROOT_OFFSET, "/", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_MESSAGE(IS_TYPE(ROMFS_TYPE_DIRECTORY, node.mode), "Root should be a directory");
}

TEST(nodes, FindEntryFollowHardlink)
{
    nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm_empty, ROOT_OFFSET, ".", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_MESSAGE(IS_TYPE(ROMFS_TYPE_DIRECTORY, node.mode), "Root should be a directory");
}

TEST_GROUP_RUNNER(nodes)
{
    RUN_TEST_CASE(nodes, GetNodeHdrBadSize);
    RUN_TEST_CASE(nodes, GetNodeHdrCheckData);

    RUN_TEST_CASE(nodes, FindEntryRoot);
    RUN_TEST_CASE(nodes, FindEntryFollowHardlink);
}

/***************************************/
TEST_GROUP(search);
/***************************************/

TEST_SETUP(search)
{
    rm_basic.img = basic_romfs;
    rm_basic.size = basic_romfs_len;
    rm_basic.vol.size = 288;
    rm_basic.vol.rootOff = ROOT_OFFSET;
}

TEST_TEAR_DOWN(search)
{
    memset(&rm_basic, 0, sizeof(rm_empty));
}

TEST(search, SearchDirNotFound)
{
    int ret;
    uint32_t offset = ROOT_OFFSET;

    ret = RomfsSearchDir(&rm_basic, "not_a_file", &offset);
    TEST_ASSERT_EQUAL_INT(-ENOENT, ret);
}

TEST(search, SearchDirFoundFile)
{
    int ret;
    uint32_t offset = ROOT_OFFSET;

    ret = RomfsSearchDir(&rm_basic, "a", &offset);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(A_FILE_OFFSET, offset);
}

TEST(search, SearchDirFoundFileInDir)
{
    int ret;
    uint32_t offset = FIRST_ENTRY_IN_DIR;

    ret = RomfsSearchDir(&rm_basic, "b", &offset);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(B_FILE_OFFSET, offset);
}

TEST_GROUP_RUNNER(search)
{
    RUN_TEST_CASE(search, SearchDirNotFound);
    RUN_TEST_CASE(search, SearchDirFoundFile);
    RUN_TEST_CASE(search, SearchDirFoundFileInDir);
    RUN_TEST_CASE(find, FindEntryFileNotFound);
    RUN_TEST_CASE(find, FindEntryRootDir);
    RUN_TEST_CASE(find, FindEntryFileFound);
    RUN_TEST_CASE(find, FindEntryFileInDirFound);
    RUN_TEST_CASE(find, FindEntryDirAbsolutePath);
    RUN_TEST_CASE(find, FindEntryDirRelativePath);
}

/***************************************/
TEST_GROUP(find);
/***************************************/

TEST_SETUP(find)
{
    rm_basic.img = basic_romfs;
    rm_basic.size = basic_romfs_len;
    rm_basic.vol.size = 288;
    rm_basic.vol.rootOff = ROOT_OFFSET;
}

TEST_TEAR_DOWN(find)
{
    memset(&rm_basic, 0, sizeof(rm_empty));
}

TEST(find, FindEntryFileNotFound)
{
    nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm_basic, ROOT_OFFSET, "not_a_file", &node);
    TEST_ASSERT_EQUAL_INT(-ENOENT, ret);

    ret = RomfsFindEntry(&rm_basic, ROOT_OFFSET, "/...", &node);
    TEST_ASSERT_EQUAL_INT(-ENOENT, ret);

    ret = RomfsFindEntry(&rm_basic, ROOT_OFFSET, "dir/not_a_file", &node);
    TEST_ASSERT_EQUAL_INT(-ENOENT, ret);
}

TEST(find, FindEntryRootDir)
{
    nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm_basic, ROOT_OFFSET, ".", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(ROOT_OFFSET, node.off);

    ret = RomfsFindEntry(&rm_basic, ROOT_OFFSET, "/", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(ROOT_OFFSET, node.off);

    ret = RomfsFindEntry(&rm_basic, ROOT_OFFSET, "./.", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(ROOT_OFFSET, node.off);

    ret = RomfsFindEntry(&rm_basic, ROOT_OFFSET, "/..", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(ROOT_OFFSET, node.off);

    ret = RomfsFindEntry(&rm_basic, ROOT_OFFSET, "/dir/../dir/..", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(ROOT_OFFSET, node.off);
}

TEST(find, FindEntryFileFound)
{
    nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm_basic, ROOT_OFFSET, "a", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(A_FILE_OFFSET, node.off);
}

TEST(find, FindEntryFileInDirFound)
{
    nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm_basic, ROOT_OFFSET, "dir/b", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(B_FILE_OFFSET, node.off);
}

TEST(find, FindEntryDirAbsolutePath)
{
    nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm_basic, ROOT_OFFSET, "/dir", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(DIR_OFFSET, node.off);
}

TEST(find, FindEntryDirRelativePath)
{
    nodehdr_t node;
    int ret;

    ret = RomfsFindEntry(&rm_basic, DIR_OFFSET, "b", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(B_FILE_OFFSET, node.off);

    ret = RomfsFindEntry(&rm_basic, DIR_OFFSET, "/b", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(B_FILE_OFFSET, node.off);

    ret = RomfsFindEntry(&rm_basic, DIR_OFFSET, "../a", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(A_FILE_OFFSET, node.off);

    ret = RomfsFindEntry(&rm_basic, DIR_OFFSET, "../dir/.././a", &node);
    TEST_ASSERT_EQUAL_INT(0, ret);

    TEST_ASSERT_EQUAL_HEX(A_FILE_OFFSET, node.off);
}

TEST_GROUP_RUNNER(find)
{
    RUN_TEST_CASE(find, FindEntryFileNotFound);
    RUN_TEST_CASE(find, FindEntryRootDir);
    RUN_TEST_CASE(find, FindEntryFileFound);
    RUN_TEST_CASE(find, FindEntryFileInDirFound);
    RUN_TEST_CASE(find, FindEntryDirAbsolutePath);
    RUN_TEST_CASE(find, FindEntryDirRelativePath);
}
