#ifndef __DOOTSTR_INC
#define __DOOTSTR_INC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOOTFAIL(source) (perror(source), fprintf(stderr, "DOOTFAIL: %s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#ifdef __DOOTSTR_DEBUG
#define DOOT_LOG_ALLOC(oldcap, newcap) (printf("Reallocating from: %ld to %ld\n", oldcap, newcap))
#else
#define DOOT_LOG_ALLOC(oldcap, newcap) 
#endif

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
    size_t capacity; /*Current size of the allocated memory block*/

} dootstr_t;

#pragma region ALLOCATION
/*
@brief Reallocates the memory block of the dootstr, copying the old contents. If the new capacity is to small to contain the old contents,
some data will be lost, however the null terminator will always be inserted.
*/
void doot_realloc(dootstr_t *pdoot, size_t newcap)
{
    DOOT_LOG_ALLOC(pdoot->capacity, newcap);
    if (newcap == 0)
    {
        DOOTFAIL("doot_realloc: Capacity of 0 is not allowed.");
    }
    if (!pdoot)
    {
        DOOTFAIL("doot_realloc: The address of dootstr pointer was null.");
    }
    pdoot->pstr = (char*)realloc(pdoot->pstr, newcap);
    if (!pdoot->pstr)
    {
        DOOTFAIL("realloc");
    }
    pdoot->capacity = newcap;
    if (pdoot->capacity < pdoot->strlen + 1) // Need to insert new null terminator
    {
        pdoot->pstr[pdoot->capacity - 1] = '\0';
        pdoot->strlen = pdoot->capacity - 1;
    }
}

#define DOOT_NEWCAPACITY(oldcap) ((oldcap)*2U)

dootstr_t *doot_newfrom(const char *cstring)
{
    dootstr_t *pdoot = (dootstr_t*)malloc(sizeof(dootstr_t));
    if (!pdoot)
    {
        DOOTFAIL("malloc");
    }
    pdoot->pstr = strdup(cstring);
    if (!pdoot->pstr)
    {
        DOOTFAIL("strdup");
    }
    pdoot->strlen = strlen(cstring);
    pdoot->capacity = pdoot->strlen;
    return pdoot;
}

dootstr_t *doot_new(size_t capacity)
{
    dootstr_t *pdoot = (dootstr_t*)malloc(sizeof(dootstr_t));
    if (!pdoot)
    {
        DOOTFAIL("malloc");
    }
    pdoot->strlen = 0;
    pdoot->pstr = NULL;
    pdoot->capacity = capacity;
    if (pdoot->capacity != 0)
    {
        pdoot->pstr = (char*)malloc(sizeof(char)*pdoot->capacity);
        if (!pdoot->pstr)
        {
            DOOTFAIL("malloc");
        }
    }
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
        if ((*ppdoot)->pstr)
        {
            free((*ppdoot)->pstr);
        }
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
    pdoot->capacity = 0;
}

#pragma endregion

#pragma region BASIC_FUNCTIONS

void doot_clear(dootstr_t *pdoot)
{
    doot_destroy(pdoot);
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
    pdoot->capacity = pdoot->strlen + 1;
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
    pleft->capacity = pleft->strlen + 1;
    if (!pleft->pstr)
    {
        DOOTFAIL("strdup");
    }
}

void doot_append_c(dootstr_t *pdoot, const char *cstring)
{
    if (!pdoot || !cstring)
    {
        DOOTFAIL("doot_append: The address of a dootstr or a c string was null.");
    }
    size_t rlen = strlen(cstring);
    if (!pdoot->pstr)
    {
        doot_realloc(pdoot, DOOT_NEWCAPACITY(rlen + 1));
        pdoot->strlen = 0; // It should be zero already, just making sure tho
        if (!pdoot->pstr)
        {
            DOOTFAIL("realloc");
        }
    }
    else if (pdoot->capacity < pdoot->strlen + rlen + 1)
    {
        doot_realloc(pdoot, DOOT_NEWCAPACITY(pdoot->strlen + rlen + 1));
        if (!pdoot->pstr)
        {
            DOOTFAIL("realloc");
        }
    }

    memcpy(pdoot->pstr + pdoot->strlen, cstring, rlen + 1);
    pdoot->strlen = pdoot->strlen + rlen; 
}

void doot_append(dootstr_t *pleft, const dootstr_t *pright)
{
    if (!pleft || !pright)
    {
        DOOTFAIL("doot_append: The address of a dootstr was null.");
    }
    if (!pright->pstr)
    {
        return;
    }
    if (!pleft->pstr)
    {
        doot_realloc(pleft, DOOT_NEWCAPACITY(pright->strlen + 1));
        pleft->strlen = 0; // It should be zero already, just making sure tho
        if (!pleft->pstr)
        {
            DOOTFAIL("realloc");
        }
    }
    else if (pleft->capacity < pleft->strlen + pright->strlen + 1)
    {
        doot_realloc(pleft, DOOT_NEWCAPACITY(pleft->strlen + pright->strlen + 1));
        if (!pleft->pstr)
        {
            DOOTFAIL("realloc");
        }
    }
    memcpy(pleft->pstr + pleft->strlen, pright->pstr, pright->strlen + 1);
    pleft->strlen = pleft->strlen + pright->strlen;
}

/*
@brief Inserts a cstring starting at a given position in the dootstr object. If the dootstr is empty only position 0 is valid.
@param[in] position it's a signed to prevent overflow when decrementing, but it shouldn't be negative.
*/
void doot_insert_c(dootstr_t *pdoot, const char *cstring, ssize_t position)
{
    if (position < 0)
    {
        DOOTFAIL("doot_insert_c: The position cannot be negative.");
    }
    if (!pdoot || !cstring)
    {
        DOOTFAIL("doot_insert_c: The address of a dootstr or a c string was null.");
    }
    if (position > pdoot->strlen)
    {
        DOOTFAIL("doot_insert_c: Position has to be no greater than string length.");
    }
    if (position == pdoot->strlen)
    {
        puts("I");
        doot_append_c(pdoot, cstring);
        return;
    }
    size_t rlen = strlen(cstring); 
    if ((!pdoot->pstr || pdoot->strlen == 0) && position != 0)
    {
        DOOTFAIL("doot_insert_c: Cannot insert at a non zero position to an empty string.");
    }
    // Block is empty - allocating new block //
    if (!pdoot->pstr)
    {
        doot_realloc(pdoot, DOOT_NEWCAPACITY(rlen + 1));
        memcpy(pdoot->pstr, cstring, rlen + 1);
        pdoot->strlen = rlen;
        return;
    }
    // Block is too small, allocating new block and manually moving //
    if (pdoot->capacity < pdoot->strlen + rlen + 1)
    {
        puts("II");
        size_t newcap = DOOT_NEWCAPACITY(pdoot->strlen + rlen + 1);
        DOOT_LOG_ALLOC(pdoot->capacity, newcap);
        char *newblock = (char*)malloc(sizeof(char)*newcap);  
        if (!newblock)
        {
            DOOTFAIL("malloc");
        }
        memcpy(newblock, pdoot->pstr, position);
        memcpy(newblock + position, cstring, rlen);
        memcpy(newblock + position + rlen, pdoot->pstr + position, pdoot->strlen - position);
        newblock[pdoot->strlen + rlen] = '\0';
        free(pdoot->pstr);
        pdoot->pstr = newblock;
        pdoot->strlen = pdoot->strlen + rlen;
        pdoot->capacity = newcap;
        return;
    }
    // Block is big enough, just moving characters around //
    // Ok so there was a horrendous bug involving overflow when comparing an int to an ssize_t.
    // Since then the index and position are of type ssize_t.
    puts("III");
    for (ssize_t i = pdoot->strlen; i >= position; --i)
    {
        pdoot->pstr[i + rlen] = pdoot->pstr[i];
    }
    memcpy(pdoot->pstr + position, cstring, rlen);
    pdoot->strlen = pdoot->strlen + rlen;
    pdoot->pstr[pdoot->strlen] = '\0';
}

void doot_insert(dootstr_t *pleft, dootstr_t *pright, size_t position)
{

}

/*
@brief Allocates a new doostr object containing the concatenated string. Not sure why someone would use this, but ok.
*/
dootstr_t *doot_concat(dootstr_t *pleft, dootstr_t *pright)
{
    if (!pleft || !pright)
    {
        DOOTFAIL("doot_concat: One of the argument dootstrs was null.");
    }
    dootstr_t *pdoot = doot_new(DOOT_NEWCAPACITY(pleft->strlen + pright->strlen + 1));
    // From what I've read, calling memcpy(_, NULL, 0) could violate the standard, hence the check
    if (pleft->pstr)
    {
        memcpy(pdoot->pstr, pleft->pstr, pleft->strlen); 
    }
    if (pright->pstr)
    {
        memcpy(pdoot->pstr + pleft->strlen, pright->pstr, pright->strlen);
    }
    pdoot->pstr[pleft->strlen + pright->strlen] = '\0';
    return pdoot;
}

// dootstr_t *doot_slice(dootstr_t *pdoot, ssize_t begin, ssize_t step, ssize_t end)
// {

// }

typedef struct dootview
{
    int x;
    // Struct that contains state of a dootstr search, view, tokenization or alike
} dootview_t;

/* HOW IT'S MEANT TO BE USED:
dootview_t view;
doot_setview(&view, dootstr_t string);
for ...
    dootstr_t *token = doot_nextok(&view, delimset)

*/


#pragma endregion

#endif