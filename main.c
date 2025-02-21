#include <stdio.h>
#include "dootstr.c"
#include <stdint.h>

typedef struct foo
{
    int x;
    char name[32];
    size_t size;
} FooType;



int main()
{
    str_t *s = str_newfrom("a");

    str_assign_c(s, "baabaaa");
    printf("%ld\n", __str_countSplits(s, "aa", NULL));

    str_free(&s);
    return EXIT_SUCCESS;
}