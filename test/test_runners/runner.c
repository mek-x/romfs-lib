#include "unity_fixture.h"

#include "../empty_romfs.h"
#include "../basic_romfs.h"

static void runAllTests(void)
{
    RUN_TEST_GROUP(volume);
    RUN_TEST_GROUP(nodes);
    RUN_TEST_GROUP(path);

    RUN_TEST_GROUP(load);
    RUN_TEST_GROUP(stat);
    RUN_TEST_GROUP(open);
    RUN_TEST_GROUP(close);
    RUN_TEST_GROUP(readFile);
    RUN_TEST_GROUP(readDir);
    RUN_TEST_GROUP(mapFile);
}

int main(int argc, const char* argv[])
{
    return UnityMain(argc, argv, runAllTests);
}
