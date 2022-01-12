#include "unity_fixture.h"

TEST_GROUP_RUNNER(load)
{
    RUN_TEST_CASE(load, basicRomfsLoad);
}

static void runAllTests(void)
{
    RUN_TEST_GROUP(load);
}

int main(int argc, const char* argv[])
{
    return UnityMain(argc, argv, runAllTests);
}

