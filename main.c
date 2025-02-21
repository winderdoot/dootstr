#include <stdio.h>
#include <wchar.h>

#define DOOTSTR_USE_WCHAR
#include "dootstr.c"

int main()
{
    str_t *s = str_newfrom(L"aaaaaaa");
    printf("%ld\n", str_count(s, L"a"));

    wprintf(L"%ls\n", s->pstr);

    str_free(&s);
    return EXIT_SUCCESS;
}