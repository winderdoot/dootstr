#ifndef __DOOTSTR_INC
#define __DOOTSTR_INC

#include <stdio.h>
#include <stdlib.h>

#define DOOTFAIL(source) (perror(source), fprintf(stderr, "DOOTFAIL: %s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

/** @struct dootstr
 *  @brief This structure wraps a raw C style char pointer to provide a more convenient interface for string manipulation.
 *  Structures of this type are to be passed to dootstr functions. The dootstr struct always allocates it's own memory and
 *  keeps ownership of it's memory. The raw char pointer can be passed to standard library functions but, the pointer shouldn't be freed
 *  nor reallocated.
 */
typedef struct dootstr
{
    char *pstr; /*Null terminated pointer to the char data*/
    size_t strlen; /*Number of stored readable characters*/

} dootstr_t;

#pragma region BASIC_FUNCTIONS

dootstr_t *doot_new()
{
    dootstr_t *pdoot = (dootstr_t*)malloc(sizeof(dootstr_t));
    if (!pdoot)
    {
        DOOTFAIL("malloc");
    }
    pdoot->pstr = NULL;
    pdoot->strlen = 0;
    return pdoot;
}

void doot_free(dootstr_t **ppdoot)
{
    if (!ppdoot)
    {
        DOOTFAIL("doot_free: The address of a dootstr pointer variable was null. Remember to pass an address (&variable) of a pointer variable to this function.");
    }
    if (*ppdoot)
    {
        free(*ppdoot);
    }
    *ppdoot = NULL;
}

void doot_assign_c(dootstr_t *pdoot, const char *cstring)
{
    if (!cstring)
    {
        DOOTFAIL("doot_assign: Cannot assign to null char pointer.");
    }
    if (!pdoot)
    {
        DOOTFAIL("doot_assign: The address of a dootstr was null.");
    }
    if (pdoot->pstr)
    {
        if (pdoot->pstr == cstring)
        {
            return;
        }
        free(pdoot->pstr);
    }
    
    pdoot->strlen = strlen(cstring);
    pdoot->pstr = strdup(cstring);
    if (!pdoot->pstr)
    {
        DOOTFAIL("strdup");
    }
}

#pragma endregion

#endif