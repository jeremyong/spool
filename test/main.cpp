#include <cassert>
#include <cstdio>
#include <spool.h>

const char* a = SP("hello");
const char* b = SP("world");
extern const char* c;
extern const char* d;
extern const char* e;
extern const char* lib1_a;
extern const char* lib1_b;
extern const char* lib1_c;
extern const char* lib1_d;

int main(int argc, char** argv)
{
    printf("%d\n", SPOOL_ID);
    printf("%x\n", a);
    printf("%x\n", b);
    printf("%x\n", c);
    printf("%x\n", d);
    printf("%x\n", e);

    printf("%s\n", a);
    printf("%s\n", b);
    printf("%s\n", c);
    printf("%s\n", d);
    printf("%s\n", e);

    assert(a == c);
    assert(a == d);
    assert(a != b);
    assert(a == e);
    assert(lib1_a == a);

    assert(lib1_b == lib1_c);
    assert(lib1_b == lib1_d);
    assert(lib1_a != lib1_d);

    return 0;
}
