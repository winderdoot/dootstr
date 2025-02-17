#include <stdio.h>
#include "dootstr.c"

#define N 2

int main()
{
    char *names[N] = {"Michael", "Emily"};
    char **strings = (char**)malloc(sizeof(char*) * N);
    // No checking fuck it
    for (int i = 0; i < N; i ++)
    {
        strings[i] = strdup(names[i]);
    }

    sarr_t *ps = str_afrom(strings, N);
    for (int i = 0; i < N; i ++)
    {
        puts(ps->strArr[i]->pstr);
    }
    for (int i = 0; i < N; i ++)
    {
        free(strings[i]);
    }
    free(strings);
    str_afree(&ps);
    return EXIT_SUCCESS;
}