#include <stdio.h>
#include "dootstr.c"

int main()
{
    str_t *s1 = str_newfrom("aabaa");
    printf("%ld\n", str_rindex(s1, "aa"));
    str_free(&s1);
    
    return EXIT_SUCCESS;
}