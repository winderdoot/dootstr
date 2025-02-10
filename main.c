#include <stdio.h>
#include "dootstr.c"

int main()
{
    dootstr_t *s1 = doot_new(20);
    doot_assign_c(s1, "123456789abc");
    printf("%s has length: %ld\n", s1->pstr, s1->strlen);

    doot_realloc(s1, 1000);
    printf("%s\n", s1->pstr);
    doot_realloc(s1, 5);
    printf("%s\n", s1->pstr);

    doot_free(&s1);
}