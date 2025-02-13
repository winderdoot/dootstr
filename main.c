#include <stdio.h>
#include "dootstr.c"

int main()
{
    str_t *s1 = str_newslice("eovdedn", STR_FROMEND(7), STR_FROMEND(0), 2);
    str_t *s2 = str_newslice("eovdedn", 1, STR_END, 2);
    puts(s1->pstr);
    puts(s2->pstr);
    str_free(&s1);
    str_free(&s2);
    return EXIT_SUCCESS;
}