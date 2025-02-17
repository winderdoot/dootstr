#include <stdio.h>
#include "dootstr.c"

#define N 2

int main()
{
    str_t *s = str_newfrom("Hello lalalal this is a very long string that will cause an overflow under the specific max size I've set up. Let's add some more characters just to make sure.");
    str_free(&s);
    return EXIT_SUCCESS;
}