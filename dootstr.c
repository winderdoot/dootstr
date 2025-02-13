/*
https://github.com/winderdoot/dootstr
Simple C string library by winderdoot.
The creator is not responsible for any memory leaks caused.
It works on my machine.
*/

#ifndef __DOOTSTR_INC
#define __DOOTSTR_INC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STRFAIL(source) (perror(source), fprintf(stderr, "STRFAIL: %s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#ifdef __DOOTSTR_DEBUG
#define STR_LOG_ALLOC(oldcap, newcap) (printf("Reallocating from: %ld to %ld\n", oldcap, newcap))
#else
#define STR_LOG_ALLOC(oldcap, newcap) 
#endif

#ifdef __DOOTSTR_SLICE_ERRORS
#define STR_SLICE_ERROR(message) (fprintf(stderr, "STRFAIL: %s:%d\n%s\n", __FILE__, __LINE__, message), exit(EXIT_FAILURE))
#else
#define STR_SLICE_ERROR(message)
#endif

#define STR_END ((size_t)1 << (__SIZE_WIDTH__ - 1)) // This is fucking trash
#define STR_FROMEND(ind) (STR_END | (size_t)ind)    // the most significant bit indicates that the index is from the back

#define STR_NEWCAPACITY(oldcap) ((oldcap)*2U)

/** @struct str_t
 *  @brief This structure wraps a raw C style char pointer and provides a dynamic string implementation.
 *  Structures of this type are to be passed to str_* functions. The str_t struct always allocates it's own memory and
 *  keeps ownership of it's memory. The raw char pointer can be passed to standard library functions but, the pointer shouldn't be freed
 *  nor reallocated manually by the user.
 */
typedef struct str
{
    char *pstr; /*Null terminated pointer to the char data*/
    size_t strlen; /*Number of stored readable characters*/
    size_t capacity; /*Current size of the allocated memory block*/

} str_t;

#pragma region ALLOCATION
/*
@brief Reallocates the memory block of the str_t, copying the old contents. If the new capacity is to small to contain the old contents,
some data will be lost, however the null terminator will always be inserted.
*/
void str_realloc(str_t *pstr, size_t newcap)
{
    STR_LOG_ALLOC(pstr->capacity, newcap);
    if (newcap == 0)
    {
        STRFAIL("str_realloc: Capacity of 0 is not allowed.");
    }
    if (!pstr)
    {
        STRFAIL("str_realloc: The address of str_t pointer was null.");
    }
    pstr->pstr = (char*)realloc(pstr->pstr, newcap);
    if (!pstr->pstr)
    {
        STRFAIL("realloc");
    }
    pstr->capacity = newcap;
    if (pstr->capacity < pstr->strlen + 1) // Need to insert new null terminator
    {
        pstr->pstr[pstr->capacity - 1] = '\0';
        pstr->strlen = pstr->capacity - 1;
    }
}

/*@brief Returns a pointer to a string initialized with a c-style string literal.*/
str_t *str_newfrom(const char *cstring)
{
    str_t *pstr = (str_t*)malloc(sizeof(str_t));
    if (!pstr)
    {
        STRFAIL("malloc");
    }
    pstr->pstr = strdup(cstring);
    if (!pstr->pstr)
    {
        STRFAIL("strdup");
    }
    pstr->strlen = strlen(cstring);
    pstr->capacity = pstr->strlen + 1;
    return pstr;
}

/*@brief Returns a pointer to a newly initialized empty string with a memory block of a given capacity.*/
str_t *str_new(size_t capacity)
{
    str_t *pstr = (str_t*)malloc(sizeof(str_t));
    if (!pstr)
    {
        STRFAIL("malloc");
    }
    pstr->strlen = 0;
    pstr->pstr = NULL;
    pstr->capacity = capacity;
    if (pstr->capacity != 0)
    {
        pstr->pstr = (char*)malloc(sizeof(char)*pstr->capacity);
        if (!pstr->pstr)
        {
            STRFAIL("malloc");
        }
    }
    return pstr;
}

// TODO: Maybe redesign this hot garbage function and change all int types to fixed width??? No? Ok. Fine.
/*@brief Returns a pointer to a new string populated with characters from the python-like slice [beg, ..., end) with a given step.
A negative step means that the order will be reversed. Use STR_END to indicate the item one after the last one. Use STR_FROMEND(n) to indicate the
n-th item from the back, ex. STR_FROMEND(1) is the last item etc.
See also 'https://stackoverflow.com/questions/509211/how-slicing-in-python-works' for more information.
Invalid bounds that would cause the string to be empty, by default do not cause en error, unless __DOOTSTR_SLICE_ERRORS is defined.
Otherwise, the function treats you like a baby and corrects your bounds to fit inside the string.
NOTE: values of beg/end cannot have the most significant bit, set to 1.*/
str_t *str_newslice(const char *cstring, size_t beg, size_t end, long step)
{
    if (!cstring)
    {
        STRFAIL("str_newslice: The passed address of the cstring is null.");
    }
    if (!(*cstring))
    {
        STR_SLICE_ERROR("str_newslice: The sliced cstring is empty.");
        return str_newfrom("");
    }

    size_t clen = strlen(cstring);
    // beg parameter
    if (beg == STR_END)
    {
        beg = clen - 1;
    }
    else if (beg & STR_END) // It's counted from the end
    {
        //puts("from behind bitch beg");
        beg = (beg & ~STR_END);
        if (beg > clen)
        {
            STR_SLICE_ERROR("str_newslice: Beg goes out of bounds from the left side.");
            beg = 0;
        }
        else 
        {
            beg = clen - beg;
        }
    }
    else if (beg > clen)
    {
        STR_SLICE_ERROR("str_newslice: Beg goes out of bounds from the right side.");
        beg = clen - 1;
    }
    // end parameter
    if (end == STR_END)
    {
        end = clen - 1;
    }
    else if (end & STR_END) // It's counted from the end
    {
        //puts("from behind bitch end");
        end = (end & ~STR_END);
        if (end > clen)
        {
            STR_SLICE_ERROR("str_newslice: End goes out of bounds from the left side.");
            end = clen - 1;
        }
        else
        {
            end = clen - end;
        }
    }
    else if (end > clen)
    {
        STR_SLICE_ERROR("str_newslice: End goes out of bounds from the right side.");
        end = clen - 1;
    }

    if (end < beg)
    {
        STR_SLICE_ERROR("str_newsloce: End is less than beg - resulting slice is invalid (empty).");
        return str_newfrom("");
    }

    printf("Beg: %ld, end: %ld, step: %ld\n", beg, end, step);
    return NULL;
    // size_t sliceLen = 0;
    // if (step > 0)
    // {

    // }
}

/*@brief Create a new doostr by taking ownership of an existing heap allocated c-style string. Other means of construction are
preferable, this is only for unique occasions.*/
str_t *str_steal(char *cstring)
{
    str_t *pstr = (str_t*)malloc(sizeof(str_t));
    if (!pstr)
    {
        STRFAIL("malloc");
    }
    pstr->pstr = cstring;
    pstr->strlen = strlen(cstring);
    pstr->capacity = pstr->strlen + 1;
    return pstr;
}

void str_free(str_t **ppstr)
{
    if (!ppstr)
    {
        STRFAIL("str_free: The address of a str_t pointer variable was null. Remember to pass an address (&variable) of a pointer variable to this function.");
    }
    if (*ppstr)
    {
        if ((*ppstr)->pstr)
        {
            free((*ppstr)->pstr);
        }
        free(*ppstr);
    }
    *ppstr = NULL;
}

/*
This function is only to be used on stack allocated str_t objects. It handles deallocating their memory. 
@param[in] pstr A pointer to a stack allocated str_t to be destroyed.
*/
void str_destroy(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_destroy: The address of a str_t was null. Cannot destroy it.");
    }
    if (pstr->pstr)
    {
        free(pstr->pstr);
    }
    pstr->strlen = 0;
    pstr->capacity = 0;
}

#pragma endregion

#pragma region MODIFICATION

void str_clear(str_t *pstr)
{
    str_destroy(pstr);
}

void str_assign_c(str_t *pstr, const char *cstring)
{
    if (!cstring)
    {
        STRFAIL("str_assign_c: Cannot assign to null char pointer.");
    }
    if (!pstr)
    {
        STRFAIL("str_assign_c: The address of a str_t was null.");
    }
    if (pstr->pstr)
    {
        if (pstr->pstr == cstring)
        {
            return;
        }
        free(pstr->pstr);
    }
    
    pstr->strlen = strlen(cstring);
    pstr->pstr = strdup(cstring);
    pstr->capacity = pstr->strlen + 1;
    if (!pstr->pstr)
    {
        STRFAIL("str_assing_c: strdup");
    }
}

void str_assign(str_t *pleft, const str_t *pright)
{
    if (!pright)
    {
        STRFAIL("str_assign: Cannot assign a null str_t to another.");
    }
    if (!pleft)
    {
        STRFAIL("str_assign: The address of the left str_t was null.");
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
        STRFAIL("strdup");
    }
}

void str_append_c(str_t *pstr, const char *cstring)
{
    if (!pstr || !cstring)
    {
        STRFAIL("str_append: The address of a str_t or a c string was null.");
    }
    size_t rlen = strlen(cstring);
    if (!pstr->pstr)
    {
        str_realloc(pstr, STR_NEWCAPACITY(rlen + 1));
        pstr->strlen = 0; // It should be zero already, just making sure tho
        if (!pstr->pstr)
        {
            STRFAIL("realloc");
        }
    }
    else if (pstr->capacity < pstr->strlen + rlen + 1)
    {
        str_realloc(pstr, STR_NEWCAPACITY(pstr->strlen + rlen + 1));
        if (!pstr->pstr)
        {
            STRFAIL("realloc");
        }
    }

    memcpy(pstr->pstr + pstr->strlen, cstring, rlen + 1);
    pstr->strlen = pstr->strlen + rlen; 
}

void str_append(str_t *pleft, const str_t *pright)
{
    if (!pleft || !pright)
    {
        STRFAIL("str_append: The address of a str_t was null.");
    }
    if (!pright->pstr)
    {
        return;
    }
    if (!pleft->pstr)
    {
        str_realloc(pleft, STR_NEWCAPACITY(pright->strlen + 1));
        pleft->strlen = 0; // It should be zero already, just making sure tho
        if (!pleft->pstr)
        {
            STRFAIL("realloc");
        }
    }
    else if (pleft->capacity < pleft->strlen + pright->strlen + 1)
    {
        str_realloc(pleft, STR_NEWCAPACITY(pleft->strlen + pright->strlen + 1));
        if (!pleft->pstr)
        {
            STRFAIL("realloc");
        }
    }
    memcpy(pleft->pstr + pleft->strlen, pright->pstr, pright->strlen + 1);
    pleft->strlen = pleft->strlen + pright->strlen;
}

/*
@brief Inserts a cstring starting at a given position in the str_t object. If the str_t is empty only position 0 is valid.
*/
void str_insert_c(str_t *pstr, const char *cstring, size_t position)
{
    // if (position < 0)
    // {
    //     DOOTFAIL("str_insert_c: The position cannot be negative.");
    // }
    if (!pstr || !cstring)
    {
        STRFAIL("str_insert_c: The address of a str_t or a c string was null.");
    }
    if (position > pstr->strlen)
    {
        STRFAIL("str_insert_c: Position has to be no greater than string length.");
    }
    if (position == pstr->strlen)
    {
        str_append_c(pstr, cstring);
        return;
    }
    size_t rlen = strlen(cstring); 
    if ((!pstr->pstr || pstr->strlen == 0) && position != 0)
    {
        STRFAIL("str_insert_c: Cannot insert at a non zero position to an empty string.");
    }
    // Block is empty - allocating new block //
    if (!pstr->pstr)
    {
        str_realloc(pstr, STR_NEWCAPACITY(rlen + 1));
        memcpy(pstr->pstr, cstring, rlen + 1);
        pstr->strlen = rlen;
        return;
    }
    // Block is too small, allocating new block and manually moving //
    if (pstr->capacity < pstr->strlen + rlen + 1)
    {
        size_t newcap = STR_NEWCAPACITY(pstr->strlen + rlen + 1);
        STR_LOG_ALLOC(pstr->capacity, newcap);
        char *newblock = (char*)malloc(sizeof(char)*newcap);  
        if (!newblock)
        {
            STRFAIL("malloc");
        }
        memcpy(newblock, pstr->pstr, position);
        memcpy(newblock + position, cstring, rlen);
        memcpy(newblock + position + rlen, pstr->pstr + position, pstr->strlen - position);
        newblock[pstr->strlen + rlen] = '\0';
        free(pstr->pstr);
        pstr->pstr = newblock;
        pstr->strlen = pstr->strlen + rlen;
        pstr->capacity = newcap;
        return;
    }
    // Block is big enough, just moving characters around //
    // Ok so there was a horrendous bug involving overflow when comparing an int to an ssize_t.
    // Since then the index and position are of type ssize_t.
    for (ssize_t i = pstr->strlen; i >= (ssize_t)position; --i)
    {
        pstr->pstr[i + rlen] = pstr->pstr[i];
    }
    memcpy(pstr->pstr + position, cstring, rlen);
    pstr->strlen = pstr->strlen + rlen;
    pstr->pstr[pstr->strlen] = '\0';
}

void str_insert(str_t *pleft, str_t *pright, size_t position)
{
    // if (position < 0)
    // {
    //     DOOTFAIL("str_insert: The position cannot be negative.");
    // }
    if (!pleft || !pright)
    {
        STRFAIL("str_insert: The address of a str_t was null.");
    }
    if (position > pleft->strlen)
    {
        STRFAIL("str_insert: Position has to be no greater than string length.");
    }
    if (position == pleft->strlen)
    {
        str_append(pleft, pright);
        return;
    }
    if ((!pleft->pstr || pleft->strlen == 0) && position != 0)
    {
        STRFAIL("str_insert_c: Cannot insert at a non zero position to an empty string.");
    }
    // Block is empty - allocating new block //
    if (!pleft->pstr)
    {
        str_realloc(pleft, STR_NEWCAPACITY(pright->strlen + 1));
        memcpy(pleft->pstr, pright->pstr, pright->strlen + 1);
        pleft->strlen = pright->strlen;
        return;
    }
    // Block is too small, allocating new block and manually moving //
    if (pleft->capacity < pleft->strlen + pright->strlen + 1)
    {
        size_t newcap = STR_NEWCAPACITY(pleft->strlen + pright->strlen + 1);
        STR_LOG_ALLOC(pleft->capacity, newcap);
        char *newblock = (char*)malloc(sizeof(char)*newcap);  
        if (!newblock)
        {
            STRFAIL("malloc");
        }
        memcpy(newblock, pleft->pstr, position);
        memcpy(newblock + position, pright->pstr, pright->strlen);
        memcpy(newblock + position + pright->strlen, pleft->pstr + position, pleft->strlen - position);
        newblock[pleft->strlen + pright->strlen] = '\0';
        free(pleft->pstr);
        pleft->pstr = newblock;
        pleft->strlen = pleft->strlen + pright->strlen;
        pleft->capacity = newcap;
        return;
    }
    // Block is big enough, just moving characters around //
    // Ok so there was a horrendous bug involving overflow when comparing an int to an ssize_t.
    // Since then the index and position are of type ssize_t.
    for (ssize_t i = pleft->strlen; i >= (ssize_t)position; --i)
    {
        pleft->pstr[i + pright->strlen] = pleft->pstr[i];
    }
    memcpy(pleft->pstr + position, pright->pstr, pright->strlen);
    pleft->strlen = pleft->strlen + pright->strlen;
    pleft->pstr[pleft->strlen] = '\0';
}

/*
@brief Allocates a new doostr object containing the concatenated string. Not sure why someone would use this, but ok.
*/
str_t *str_concat(str_t *pleft, str_t *pright)
{
    if (!pleft || !pright)
    {
        STRFAIL("str_concat: One of the argument str_t was null.");
    }
    str_t *pstr = str_new(STR_NEWCAPACITY(pleft->strlen + pright->strlen + 1));
    // From what I've read, calling memcpy(_, NULL, 0) could violate the standard, hence the check
    if (pleft->pstr)
    {
        memcpy(pstr->pstr, pleft->pstr, pleft->strlen); 
    }
    if (pright->pstr)
    {
        memcpy(pstr->pstr + pleft->strlen, pright->pstr, pright->strlen);
    }
    pstr->pstr[pleft->strlen + pright->strlen] = '\0';
    return pstr;
}

/*
@brief Removes a substring from a str_t.
@param[in] pstr string object
@param[in] position position to where the removed substr begins
@param[in] length the length of the substring to remove. Must be at least 1.
NOTE: As of currently I can't be bothered to implement safety measures against potential overflow
caused by ex. position + length. Fix it later.
*/
void str_cut(str_t *pstr, size_t position, size_t length)
{
    if (!pstr)
    {
        STRFAIL("str_cut: The passed address was null.");
    }
    if (length < 1)
    {
        return;
    }
    if (position + 1 > pstr->strlen || position + length > pstr->strlen)
    {
        STRFAIL("str_cut: The substring to be removed goes out of bounds of the string.");
    }
    for (ssize_t i = position + length; i <= pstr->strlen; ++i) // Moving hte null terminator as well
    {
        pstr->pstr[i - length] = pstr->pstr[i];
    }
    pstr->strlen -= length;
}

size_t str_count(str_t *pstr, const char* seq); // Temporary solution to solve compilation issues.
size_t str_countAny(str_t *pstr, const char* set);

/*@brief Removes all occurances of seq from the string. Returns number of removed instances.*/
size_t str_remove(str_t *pstr, const char *seq)
{
    if (!pstr)
    {
        STRFAIL("str_remove: The passed address was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    if (!seq)
    {
        STRFAIL("str_remove: The passed address of seq was null.");
    }
    if (!(*seq))
    {
        return 0;
    }
    size_t count = str_count(pstr, seq);
    size_t rlen = strlen(seq);
    size_t newLen = pstr->strlen - count *rlen;
    size_t *seqPos = (size_t*)malloc(sizeof(size_t)*count); // Array housing the positions of found substrings
    if (!seqPos)
    {
        STRFAIL("malloc");
    }
    size_t i = 0;
    char *p = pstr->pstr;
    while ((p = strstr(p, seq)) != NULL)
    {
        //printf("pos: %ld\n", p - pstr->pstr);
        seqPos[i++] = p - pstr->pstr;
        p += rlen;
    }
    i = 0; // The current element to be moved to the left (or left alone)
    size_t seqInd = 0; // The index of the next sequence to be ecnountered
    size_t offset = 0; // The offset by which to move elements to the left
    while (i < pstr->strlen + 1) // Plus 1 to copy '/0'
    {
        //printf("i: %ld\n", i);
        if (seqInd < count && i == seqPos[seqInd])
        {
            i += rlen;
            offset += rlen;
            seqInd++;
        }
        else if (offset > 0)
        {
            pstr->pstr[i - offset] = pstr->pstr[i];
            ++i;
        }
        else
        {
            ++i;
        }
    }
    free(seqPos);
    pstr->strlen = newLen;
    //pstr->pstr[pstr->strlen-1] = '\0';
    return count;
}

/*@brief Removes all occurances of any character in set from the string. Returns number of removed instances.*/
size_t str_removeAny(str_t *pstr, const char *set)
{
    if (!pstr)
    {
        STRFAIL("str_removeAny: The passed address was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    if (!set)
    {
        STRFAIL("str_removeAny: The passed address of set was null.");
    }
    size_t count = str_countAny(pstr, set);
    size_t newLen = pstr->strlen - count;
    size_t *seqPos = (size_t*)malloc(sizeof(size_t)*count); // Array housing the positions of found substrings
    if (!seqPos)
    {
        STRFAIL("malloc");
    }
    size_t i = 0;
    char *p = pstr->pstr;
    while ((p = strpbrk(p, set)) != NULL)
    {
        //printf("pos: %ld\n", p - pstr->pstr);
        seqPos[i++] = p - pstr->pstr;
        ++p;
    }
    i = 0; // The current element to be moved to the left (or left alone)
    size_t seqInd = 0; // The index of the next sequence to be ecnountered
    size_t offset = 0; // The offset by which to move elements to the left
    while (i < pstr->strlen + 1) // Plus 1 to copy '/0'
    {
        //printf("i: %ld\n", i);
        if (seqInd < count && i == seqPos[seqInd])
        {
            offset++;
            seqInd++;
        }
        else if (offset > 0)
        {
            pstr->pstr[i - offset] = pstr->pstr[i];
        }
        i++;
    }
    free(seqPos);
    pstr->strlen = newLen;
    //pstr->pstr[pstr->strlen-1] = '\0';
    return count;
}

#pragma endregion

#pragma region LOGICAL
int str_isempty(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_isempty: The address of a str_t was null.");
    }
    return !pstr->pstr || (pstr->strlen == 0);
}

int str_isalnum(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_isalnum: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    char *p = pstr->pstr;
    while (*p)
    {
        if (!isalnum(*p))
        {
            return 0;
        }
        ++p;
    }
    return 1;
}

int str_isalpha(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_isalpha: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    char *p = pstr->pstr;
    while (*p)
    {
        if (!isalpha(*p))
        {
            return 0;
        }
        ++p;
    }
    return 1;
}

int str_isdigit(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_isdigit: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    char *p = pstr->pstr;
    while (*p)
    {
        if (!isdigit(*p))
        {
            return 0;
        }
        ++p;
    }
    return 1;
}

int str_islower(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_islower: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    char *p = pstr->pstr;
    while (*p)
    {
        if (isalpha(*p) && !islower(*p))
        {
            return 0;
        }
        ++p;
    }
    return 1;
}

int str_isupper(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_isupper: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    char *p = pstr->pstr;
    while (*p)
    {
        if (isalpha(*p) && !isupper(*p))
        {
            return 0;
        }
        ++p;
    }
    return 1;
}

int str_isspace(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_isspace: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    char *p = pstr->pstr;
    while (*p)
    {
        if (isspace(*p))
        {
            return 0;
        }
        ++p;
    }
    return 1;
}

/*Returns 1 if the string contains only characters from a given set*/
int str_containsOnly(str_t *pstr, const char *set)
{
    if (!pstr)
    {
        STRFAIL("str_containsOnly: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    char *p = pstr->pstr;
    while (*p)
    {
        if (strpbrk(p, set) != p)
        {
            return 0;
        }
        ++p;
    }
    return 1;
}

/*Returns 1 if the string contains any of the characters characters from a given set.
The function is just a wrapper for strpbrk(), but I kept it for convenience.*/
int str_containsAny(str_t *pstr, const char *set)
{
    if (!pstr)
    {
        STRFAIL("str_containsAny: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    return strpbrk(pstr->pstr, set) != NULL;
}

/*Returns 1 if the string contains seq as a substr.
The function is just a wrapper for strstr() but I kept it for convenience.*/
int str_containsSeq(str_t *pstr, const char *seq)
{
    if (!pstr)
    {
        STRFAIL("str_containsAny: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    return strstr(pstr->pstr, seq) != NULL;
}
#pragma endregion

#pragma region TRANSFORM
void str_upper(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_upper: The passed address was null.");
    }
    char *p = pstr->pstr;
    while (*p)
    {
        *p = toupper(*p); 
        ++p;
    }
}

void str_lower(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_upper: The passed address was null.");
    }
    char *p = pstr->pstr;
    while (*p)
    {
        *p = tolower(*p);
        ++p;
    }
}

void str_swapcase(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_swapcase: The passed address was null.");
    }
    if (!pstr->pstr)
    {
        return;
    }
    char *p = pstr->pstr;
    while (*p)
    {
        if (isupper(*p))
        {
            *p = tolower(*p);
        }
        else if (islower(*p))
        {
            *p = toupper(*p);
        }
        p++;
    }
}

/*@brief Counts how many times a sequence is found in a string.*/
size_t str_count(str_t *pstr, const char* seq)
{
    if (!pstr)
    {
        STRFAIL("str_count: The passed address was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    if (!seq)
    {
        STRFAIL("str_count: The passed address of seq was null.");
    }
    if (!(*seq))
    {
        STRFAIL("str_count: The passed sequence is empty.");
    }
    size_t count = 0;
    char *p = pstr->pstr;
    while ((p = strstr(p, seq)) != NULL)
    {
        count++;
        p++;
    }
    return count;
}

/*@brief Counts how many times a character from a set is found in the string.*/
size_t str_countAny(str_t *pstr, const char* set)
{
    if (!pstr)
    {
        STRFAIL("str_countAny: The passed address was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    if (!set)
    {
        STRFAIL("str_countAny: The passed address of set was null.");
    }
    if (!(*set))
    {
        STRFAIL("str_count: The passed set is empty.");
    }
    size_t count = 0;
    char *p = pstr->pstr;
    while ((p = strpbrk(p, set)) != NULL)
    {
        count++;
        p++;
    }
    return count;
}

/*@brief Replaces each full occurance of oldval with newval. Returns the number of replaced instances. Always causes reallocation.
NOTE: This *can* be used for removing substrings, but isn't recomended as this function causes an unnecessary reallocation. In such cases
use str_remove() instead!*/
size_t str_replace(str_t *pstr, const char *oldval, const char *newval)  // I suck at C, debugging this was hell...
{
    if (!pstr)
    {
        STRFAIL("str_replace: The passed address was null.");
    }
    if (!oldval)
    {
        STRFAIL("str_replace: The passed address of oldval was null.");
    }
    if (!newval)
    {
        STRFAIL("str_replace: The passed address of newval was null.");
    }
    size_t count = str_count(pstr, oldval);
    size_t rlen = strlen(newval), llen = strlen(oldval);
    size_t newLen = (rlen > llen) ? pstr->strlen + count*(rlen-llen) : pstr->strlen - count*(llen-rlen);
    size_t *offsets = (size_t*)malloc(sizeof(size_t)*count);
    if (!offsets)
    {
        STRFAIL("malloc");
    }
    size_t i = 0;
    char *p = pstr->pstr;
    while ((p = strstr(p, oldval)) != NULL)
    {
        offsets[i++] = p - pstr->pstr;
        p += llen;
    }

    char *newblock;
    size_t blocksize;
    if (pstr->capacity < newLen + 1)
    {
        blocksize = newLen + 1;
    }
    else
    {
        blocksize = pstr->capacity;
    }
    newblock = (char*)malloc(sizeof(char)*blocksize);
    memset(newblock, 0, blocksize);
    STR_LOG_ALLOC(pstr->capacity, blocksize);
    if (!newblock)
    {
        STRFAIL("malloc");
    }

    i = 0;
    size_t indOff = 0;
    size_t oldpos = 0;
    while (i < newLen + 1) // Plus 1 to copy '/0'
    {
        if (indOff < count && oldpos == offsets[indOff])
        {
            memcpy(newblock + i, newval, rlen);
            i += rlen;
            oldpos += llen;
            indOff++;
        }
        else
        {
            newblock[i++] = pstr->pstr[oldpos];
            oldpos++;
        }   
    }
    //newblock[blocksize-1] = '\0';
    free(pstr->pstr);
    free(offsets);
    pstr->pstr = newblock;
    pstr->capacity = blocksize;
    pstr->strlen = newLen;
    return count;
}

/*@brief Replaces any of the characters in set with newval. Returns the number of replaced instances. Always causes reallocation.
NOTE: This *can* be used for removing characters, but isn't recomended as this function causes an unnecessary reallocation. In such cases
use str_removeAny() instead!*/
size_t str_replaceAny(str_t *pstr, const char *set, const char *newval)
{
    if (!pstr)
    {
        STRFAIL("str_replaceAny: The passed address was null.");
    }
    if (!set)
    {
        STRFAIL("str_replaceAny: The passed address of set was null.");
    }
    if (!newval)
    {
        STRFAIL("str_replaceAny: The passed address of newval was null.");
    }
    size_t count = str_countAny(pstr, set);
    size_t rlen = strlen(newval);
    size_t extraChars = count * rlen - count; // Additional needed characters
    size_t *offsets = (size_t*)malloc(sizeof(size_t)*count);
    if (!offsets)
    {
        STRFAIL("malloc");
    }
    size_t i = 0;
    char *p = pstr->pstr;
    while ((p = strpbrk(p, set)) != NULL)
    {
        offsets[i++] = p - pstr->pstr;
        p++;
    }

    char *newblock;
    size_t blocksize;
    if (pstr->capacity < pstr->strlen + extraChars + 1)
    {
        blocksize = pstr->strlen + extraChars + 1;
    }
    else
    {
        blocksize = pstr->capacity;
    }
    newblock = (char*)malloc(sizeof(char)*blocksize);
    STR_LOG_ALLOC(pstr->capacity, blocksize);
    if (!newblock)
    {
        STRFAIL("malloc");
    }

    i = 0;
    size_t indOff = 0;
    size_t oldpos = 0;
    while (i < pstr->strlen + extraChars + 1) // Plus 1 to copy '/0'
    {
        if (indOff < count && oldpos == offsets[indOff])
        {
            memcpy(newblock + i, newval, rlen);
            i += rlen;
            indOff++;
        }
        else
        {
            newblock[i++] = pstr->pstr[oldpos];
        }
        oldpos++;
    }
    free(pstr->pstr);
    free(offsets);
    pstr->pstr = newblock;
    pstr->capacity = blocksize;
    pstr->strlen = pstr->strlen + extraChars;
    return count;
}

/*@brief Replaces any of the characters in set with a char c. Returns the number of replaced instances.*/
size_t str_replaceAnyCh(str_t *pstr, const char *set, char c)
{
    if (!pstr)
    {
        STRFAIL("str_replaceAnyCh: The passed address was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    if (!set)
    {
        STRFAIL("str_replaceAnyCh: The passed address of seq was null.");
    }
    size_t count = 0;
    char *p = pstr->pstr;
    while ((p = strpbrk(p, set)) != NULL)
    {
        *p = c;
        count++;
        p++;
    }
    return count;
}

/*@brief Removes all preceding and trailing whitespaces.*/
void str_strip(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_strip: The passed address of str_t was null.");
    }
    if (!pstr->pstr)
    {
        return;
    }
    size_t leftoff = 0, rightoff = 0;
    char *p = pstr->pstr;
    while (*p && isspace(*p))
    {
        ++leftoff;
        ++p;
    }
    if (leftoff == pstr->strlen)
    {
        pstr->pstr[0] = '\0';
        pstr->strlen = 0;
        return;
    }
    p = pstr->pstr + pstr->strlen - 1;
    while (p >= pstr->pstr && isspace(*p))
    {
        ++rightoff;
        --p;
    }
    if (leftoff > 0)
    {
        for (size_t i = leftoff; i < pstr->strlen - rightoff; ++i)
        {
            pstr->pstr[i - leftoff] = pstr->pstr[i];
        }
    }
    pstr->pstr[pstr->strlen - rightoff - leftoff] = '\0';
}

/*@brief Removes all preceding whitespaces.*/
void str_lstrip(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_strip: The passed address of str_t was null.");
    }
    if (!pstr->pstr)
    {
        return;
    }
    size_t leftoff = 0;
    char *p = pstr->pstr;
    while (*p && isspace(*p))
    {
        ++leftoff;
        ++p;
    }
    if (leftoff == pstr->strlen)
    {
        pstr->pstr[0] = '\0';
        pstr->strlen = 0;
        return;
    }
    if (leftoff > 0)
    {
        for (size_t i = leftoff; i < pstr->strlen; ++i)
        {
            pstr->pstr[i - leftoff] = pstr->pstr[i];
        }
    }
    pstr->pstr[pstr->strlen - leftoff] = '\0';
}

/*@brief Removes all trailing whitespaces.*/
void str_rstrip(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_strip: The passed address of str_t was null.");
    }
    if (!pstr->pstr)
    {
        return;
    }
    size_t rightoff = 0;
    char *p = pstr->pstr + pstr->strlen - 1;
    while (p >= pstr->pstr && isspace(*p))
    {
        ++rightoff;
        --p;
    }
    pstr->pstr[pstr->strlen - rightoff] = '\0';
}
//TODO: Investigate what other cleanup fnc could be usefull.

#pragma endregion

#pragma region SPLITTING
/*@brief Searches the string and returns the index where seq first occurs. -1 otherwise.*/
ssize_t str_index(str_t *pstr, const char *seq)
{
    if (!pstr)
    {
        STRFAIL("str_index: The passed address is null.");
    }
    if (!pstr->pstr)
    {
        return -1;
    }
    if (!seq)
    {
        STRFAIL("str_index: The passed address of seq is null.");
    }
    if (!(*seq))
    {
        return 0;
    }
    ssize_t found = -1;
    ssize_t i = 0;
    const char *ps = seq;
    while (pstr->pstr[i])
    {
        found = i;
        while (pstr->pstr[i] && pstr->pstr[i] == *ps)
        {
            ps++;
            if (!(*ps))
            {
                return found;
            }
            i++;
        }
        ps = seq;
        i = found + 1;
        continue;
        i++;
    }
    return -1;
}

/*@brief Searches the string and returns the index where seq last occurs. -1 otherwise.*/
ssize_t str_rindex(str_t *pstr, const char *seq)
{
    if (!pstr)
    {
        STRFAIL("str_index: The passed address is null.");
    }
    if (!pstr->pstr)
    {
        return -1;
    }
    if (!seq)
    {
        STRFAIL("str_rindex: The passed address of seq is null.");
    }
    if (!(*seq))
    {
        return 0;
    }
    ssize_t ending = -1;
    ssize_t i = pstr->strlen -1;
    ssize_t rlen = strlen(seq);
    const char *ps = seq + rlen - 1;
    while (i >= 0)
    {
        ending = i;
        while (i >= 0 && pstr->pstr[i] == *ps)
        {
            ps--;
            if (ps < seq)
            {
                return i;
            }
            i--;
        }
        ps = seq;
        i = ending - 1;
        continue;
        i--;
    }
    return -1;
}

/*@brief Finds the first occurance of pivot and splits the string by it.
@param[in] pstr input string (non empty)
@param[in] pivot pivot sequence
@param[in] pivot str_t containing the substring before the pivot.
@param[in] outleft str_t containing the substring before the pivot.
@param[in] outmid str_t containing the substring before the pivot.*/
void str_partition(str_t *pstr, const char *pivot, str_t **outleft, str_t **outmid, str_t **outright)
{
    if (!pstr)
    {
        STRFAIL("str_partition: The passed address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        STRFAIL("str_partition: The passed string was empty.");
    }
    if (!pivot)
    {
        STRFAIL("str_partition: The passed address of pivot was null.");
    }
    if (!(*pivot))
    {
        STRFAIL("str_partition: The passed pivot was empty.");
    }
    if (!outleft || !outmid || !outright)
    {
        STRFAIL("str_partition: One of the passed addreses of output str_t pointer variables was null.");
    }
    //size_t pivlen = strlen(pivot);
    //size_t pivInd = str_index(pstr, pivot);
    //*outleft = str_new()
}

// str_t *str_slice(str_t *pstr, ssize_t begin, ssize_t step, ssize_t end)
// {

// }

typedef struct dootview
{
    int x;
    // Struct that contains state of a str_t search, view, tokenization or alike
} dootview_t;

/* HOW IT'S MEANT TO BE USED:
dootview_t view;
str_setview(&view, str_t string);
for ...
    str_t *token = str_nextok(&view, delimset)

*/
#pragma endregion


#endif