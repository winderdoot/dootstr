#include <stdio.h>
#include "dootstr.c"

int main()
{
    dootstr_t *s1 = doot_new(0);
    
    doot_insert_c(s1, "AaA", 0);
    puts(s1->pstr);

    dootstr_t *s2 = doot_newfrom("Second string");
    doot_insert(s2, s1, 0);
    puts(s2->pstr);
    doot_insert(s2, s1, 10);
    puts(s2->pstr);

    doot_free(&s1);
    doot_free(&s2);
    
    return EXIT_SUCCESS;
}