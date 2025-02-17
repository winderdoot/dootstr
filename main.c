#include <stdio.h>
#include "dootstr.c"

int main()
{
    str_t *mess = str_newfrom("abc123def");
    str_t *s1 = str_new(0), *s2 = str_new(0), *s3 = str_new(0);
    str_partition(mess, "123", &s1, &s2, &s3);
    puts(s1->pstr);
    puts(s2->pstr);
    puts(s3->pstr);
    str_free(&s1);
    str_free(&s2);
    str_free(&s3);
    str_free(&mess);
    return EXIT_SUCCESS;
}