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

TEST(multi, a)
{

}


TEST_GROUP_RUNNER(multi)
{
    RUN_TEST_CASE(multi, a);
}
