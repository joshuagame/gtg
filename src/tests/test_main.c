#include <stdio.h>

static int test_c17_is_enabled(void)
{
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201710L
    fputs("FAIL: the test target is not compiled as ISO C17\n", stderr);
    return 1;
#else
    return 0;
#endif
}

int main(void)
{
    const int result = test_c17_is_enabled();

    if (result != 0)
    {
        return result;
    }

    puts("PASS: GTG C17 bootstrap smoke test");
    return 0;
}
