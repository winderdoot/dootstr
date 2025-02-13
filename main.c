#include <stdio.h>
#include "dootstr.c"

int main()
{
    str_t *s1 = str_newslice("Ala ma kota", 0, STR_END, 2);
    puts(s1->pstr);
    str_free(&s1);
    
    return EXIT_SUCCESS;
}