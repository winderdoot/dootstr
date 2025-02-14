#include <stdio.h>
#include "dootstr.c"

int main()
{
    str_t *s = str_newfrom("Java script is a badass language");
    str_cut(s, 20, 3);
    puts(s->pstr);
    str_free(&s);
    return EXIT_SUCCESS;
}