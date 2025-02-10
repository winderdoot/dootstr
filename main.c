#include <stdio.h>
#include "dootstr.c"

int main()
{
    dootstr_t str;
    doot_assign_c(&str, "Hello this is my string");
    printf("%s has length: %ld\n", str.pstr, str.strlen);

    doot_destroy(&str);
    dootstr_t *s1 = doot_new(20);
    doot_assign_c(s1, "10 characters");
    printf("%s has length: %ld\n", s1->pstr, s1->strlen);
    doot_free(&s1);
}