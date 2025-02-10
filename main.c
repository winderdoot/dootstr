#include <stdio.h>
#include "dootstr.c"

int main()
{
    dootstr_t str;
    doot_assign_c(&str, "Hello this is my string");
    printf("%s has length: %ld\n", str.pstr, str.strlen);

    doot_destroy(&str);
}