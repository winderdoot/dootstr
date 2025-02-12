#include <stdio.h>
#include "dootstr.c"

int main()
{
    dootstr_t *s1 = doot_newfrom("Apples apples pupa");
    for (int i = 0; i < 500; i++)
    {
        doot_append_c(s1, "Apples apples pupa");
    }
    printf("%ld\n", doot_count(s1, "pp"));

    doot_free(&s1);
    
    return EXIT_SUCCESS;
}