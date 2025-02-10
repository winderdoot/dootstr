#include <stdio.h>
#include "dootstr.c"

int main()
{
    dootstr_t *s1 = doot_new(0);
    doot_append_c(s1, " What ");
    doot_append_c(s1, "an ");
    puts(s1->pstr);
    doot_append_c(s1, "amazing library!");
    puts(s1->pstr);
    doot_append_c(s1, " I love it!");
    puts(s1->pstr);
    doot_realloc(s1, s1->strlen);
    doot_append_c(s1, "x");
    puts(s1->pstr);
    doot_free(&s1);

}