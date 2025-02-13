#include <stdio.h>
#include "dootstr.c"

int main()
{
    dootstr_t *s1 = doot_newfrom("abaabaa");
    doot_replace(s1, "a", "aa");
    printf("len: %ld, cap: %ld\n", s1->strlen, s1->capacity);
    puts(s1->pstr);


    doot_free(&s1);
    
    return EXIT_SUCCESS;
}