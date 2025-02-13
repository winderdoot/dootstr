#include <stdio.h>
#include "dootstr.c"

int main()
{
    str_t *s1 = str_newfrom("\na     ");
    str_strip(s1);
    printf("len: %ld, cap: %ld\n", s1->strlen, s1->capacity);
    printf("%s", s1->pstr);
    puts("next");
    str_free(&s1);
    
    return EXIT_SUCCESS;
}