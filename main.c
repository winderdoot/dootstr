#include <stdio.h>
#include "dootstr.c"

#define N 2

int main()
{
    str_t *s = str_newfrom("a");

    str_assign_c(s, "baabaaa");
    printf("%ld\n", __str_countSplits(s, "aa", NULL));

    str_free(&s);
    return EXIT_SUCCESS;
}