#include <stdio.h>
#include "dootstr.c"

int main()
{
    str_t *s1 = str_newslice("wtf", STR_FROMEND(2), STR_FROMEND(1), 1);
    str_free(&s1);
    
    return EXIT_SUCCESS;
}