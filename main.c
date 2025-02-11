#include <stdio.h>
#include "dootstr.c"

int main()
{
    dootstr_t *s1 = doot_new(0);
    doot_insert_c(s1, "New string", 0);
    puts(s1->pstr);
    doot_insert_c(s1, "Newer string", 0);
    puts(s1->pstr);
    doot_insert_c(s1, "The newest string", 0);
    puts(s1->pstr);
    doot_insert_c(s1, "ABCD===EEEfff", 5);
    puts(s1->pstr);
    doot_insert_c(s1, "UwU", 20);
    puts(s1->pstr);


    doot_free(&s1);
    return EXIT_SUCCESS;
}