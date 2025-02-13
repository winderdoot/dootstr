#include <stdio.h>
#include "dootstr.c"

int main()
{
    str_t *s1 = str_newfrom("AAbbaAAAv");
    str_remove(s1, "AAb");
    printf("len: %ld, cap: %ld\n", s1->strlen, s1->capacity);
    puts(s1->pstr);


    str_free(&s1);
    
    return EXIT_SUCCESS;
}