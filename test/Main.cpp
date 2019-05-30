#include <spool.h>

#include "Test.hpp"
#include <cstdio>

static size_t test_count = 0;
static size_t test_passes = 0;

void test(bool condition, const char* cond_str, const char* file, int line)
{
    ++test_count;
    if (condition)
    {
        ++test_passes;
    }
    else
    {
        printf("[%s:%d] Test failed: %s\n", file, line, cond_str);
    }
}

extern const char* tu1_foo;
extern const char* tu1_bar;
extern const char* tu1_zoo;
extern const char* lib1_x;
extern const char* lib2_x;

int main(int argc, char** argv)
{
    const char* foo = SP("super");
    const char* bar = SP("super");
    const char* zoo = SP("duper");
    TEST(foo == bar);
    TEST(foo != zoo);

    TEST(bar == tu1_foo);
    TEST(zoo == tu1_zoo);
    TEST(tu1_bar == foo);

    TEST(lib1_x == SP("x"));
    TEST(lib1_x == lib2_x);

    printf("%d out of %d tests passed.\n", test_passes, test_count);
}
