#include <stdio.h>
#include "dootstr.c"

int main()
{
    printf("%ld\n", sizeof(dootstr_t));
    printf("%ld\n", sizeof(char*));
    printf("%ld\n", sizeof(size_t));
}