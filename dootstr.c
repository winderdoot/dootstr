#ifndef __DOOTSTR_INC
#define __DOOTSTR_INC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOOTFAIL(source) (perror(source), fprintf(stderr, "DOOTFAIL: %s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

/** @struct dootstr
 *  @brief This structure wraps a raw C style char pointer and provides a dynamic string implementation.
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

/*
This function is only to be used on stack allocated dootstr objects. It handles deallocating their memory. 
@param[in] pdoot A pointer to a stack allocated dootstrto be destroyed.
*/
void doot_destroy(dootstr_t *pdoot)
{
    if (!pdoot)
    {
        DOOTFAIL("doot_destroy: The address of a dootstr was null. Cannot destroy it.");
    }
    if (pdoot->pstr)
    {
        free(pdoot->pstr);
    }
    pdoot->strlen = 0;
}

void doot_assign_c(dootstr_t *pdoot, const char *cstring)
{
    if (!cstring)
    {
        DOOTFAIL("doot_assign_c: Cannot assign to null char pointer.");
    }
    if (!pdoot)
    {
        DOOTFAIL("doot_assign_c: The address of a dootstr was null.");
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
        DOOTFAIL("doot_assing_c: strdup");
    }
}

void doot_assign(dootstr_t *pleft, const dootstr_t *pright)
{
    if (!pright)
    {
        DOOTFAIL("doot_assign: Cannot assign a null dootstring to another.");
    }
    if (!pleft)
    {
        DOOTFAIL("doot_assign: The address of the left dootstr was null.");
    }
    if (pleft == pright)
    {
        return;
    }
    if (pleft->pstr)
    {
        free(pleft->pstr);
    }
    pleft->strlen = pright->strlen;
    pleft->pstr = strdup(pright->pstr);
    if (!pleft->pstr)
    {
        DOOTFAIL("strdup");
    }
}



#pragma endregion

#endif