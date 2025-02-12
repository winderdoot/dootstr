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

    dootstr_t s2 = {};
    doot_assign_c(&s2, "Stack allocated string");
    doot_append_c(&s2, " Just added more stuuuff!\n");
    puts(s2.pstr);
    doot_insert_c(&s2, "funky damn", 13);
    puts(s2.pstr);

    doot_destroy(&s2);
    
    return EXIT_SUCCESS;
}