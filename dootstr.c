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
#include <locale.h>
#include <wchar.h>

#define STRFAIL(message) (fprintf(stderr, "STRFAIL: %s:%d\n%s\n", __FILE__, __LINE__, message), exit(EXIT_FAILURE))
// For my own functions
#define STRERROR(source ) (perror(source), fprintf(stderr, "STRFAIL: %s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
// For standard functions

#ifdef DOOTSTR_DEBUG
#define STR_LOG_ALLOC(oldcap, newcap) (printf("Reallocating from: %ld to %ld\n", oldcap, newcap))
#else
#define STR_LOG_ALLOC(oldcap, newcap) 
#endif

#ifdef DOOTSTR_SLICE_ERRORS
#define STR_SLICE_ERROR(message) (fprintf(stderr, "STRFAIL: %s:%d\n%s\n", __FILE__, __LINE__, message), exit(EXIT_FAILURE))
#else
#define STR_SLICE_ERROR(message)
#endif

#define STR_END ((size_t)1 << (__SIZE_WIDTH__ - 1)) // This is a special value indicating the position one after the last character
#define STR_FROMEND(ind) (STR_END | (size_t)ind)    // This allows you to index the string from the back
// The most significant bit indicates that the index is from the back

#define STR_MAXSIZE (2ULL << 31) // 2GB
#define STR_EXPR_TESTOVERFLOW(oldcap) (((oldcap)*2U < oldcap || (oldcap)*2U > STR_MAXSIZE) ? (STRFAIL("Memory failure: the requested memory size either exceds 2GB, or the width of size_t type.")) : 0)
#define STR_NEWCAPACITY(oldcap) ((void)STR_EXPR_TESTOVERFLOW(oldcap) , ((oldcap)*2U))
/*A little bit of cursed macro magic
This checks if the new size would be greater than a set maximum size, but also checks against overflow and throws an error if the required amount
would be greater than the width of size_t (a bit redundant, since size_t is almost always very wide).
Note that unsigned integer overflow is defined behaviour by the C standard.*/

#ifdef DOOTSTR_USE_WCHAR
typedef wchar_t dchar_t;
#define _strlen(s) (wcsnlen(s, (size_t)STR_MAXSIZE))
#define _strdup(s) wcsdup(s)
#define _strstr(s1, s2) wcsstr(s1, s2)
#define _strpbrk(s1, s2) wcspbrk(s1, s2)
#define STR_EMPTY L""
#else
typedef char dchar_t;
#define _strlen(s) (strnlen(s, (size_t)STR_MAXSIZE))
#define _strdup(s) strdup(s)
#define _strstr(s1, s2) strstr(s1, s2)
#define _strpbrk(s1, s2) strpbrk(s1, s2)
#define STR_EMPTY ""
#endif

/** @struct str_t
 *  @brief This structure wraps a raw C style char pointer and provides a dynamic string implementation.
 *  Structures of this type are to be passed to str_* functions. The str_t struct always allocates it's own memory and
 *  keeps ownership of it's memory. The raw char pointer can be passed to standard library functions but, the pointer shouldn't be freed
 *  nor reallocated manually by the user.
 */
typedef struct str
{
    dchar_t *pstr; /*Null terminated pointer to the char data*/
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
    (void)STR_EXPR_TESTOVERFLOW(newcap / 2);
    STR_LOG_ALLOC(pstr->capacity, newcap);
    if (newcap == 0)
    {
        STRFAIL("str_realloc: Capacity of 0 is not allowed.");
    }
    if (!pstr)
    {
        STRFAIL("str_realloc: The address of str_t pointer was null.");
    }
    pstr->pstr = (dchar_t *)realloc(pstr->pstr, newcap * sizeof(dchar_t));
    if (!pstr->pstr)
    {
        STRERROR("realloc");
    }
    pstr->capacity = newcap;
    if (pstr->capacity < pstr->strlen + 1) // Need to insert new null terminator
    {
        pstr->pstr[pstr->capacity - 1] = '\0';
        pstr->strlen = pstr->capacity - 1;
    }
}

/*@brief Returns a pointer to a string initialized with a c-style string literal.*/
str_t *str_newfrom(const dchar_t *cstring)
{
    str_t *pstr = (str_t*)malloc(sizeof(str_t));
    if (!pstr)
    {
        STRERROR("malloc");
    }
    pstr->strlen = _strlen(cstring);
    (void)STR_EXPR_TESTOVERFLOW((pstr->strlen + 1) / 2);
    pstr->pstr = _strdup(cstring);
    if (!pstr->pstr)
    {
        #ifdef DOOTSTR_USE_WCHAR
        STRERROR("wcsdup");
        #else
        STRERROR("strdup");
        #endif
    }  
    pstr->capacity = pstr->strlen + 1;
    return pstr;
}

/*@brief Returns a pointer to a newly initialized empty string with a memory block of a given capacity.*/
str_t *str_new(size_t capacity)
{
    str_t *pstr = (str_t*)malloc(sizeof(str_t));
    if (!pstr)
    {
        STRERROR("malloc");
    }
    pstr->strlen = 0;
    pstr->pstr = NULL;
    pstr->capacity = capacity;
    if (pstr->capacity != 0)
    {
        (void)STR_EXPR_TESTOVERFLOW(capacity / 2);
        pstr->pstr = (dchar_t *)malloc(sizeof(dchar_t) * pstr->capacity);
        if (!pstr->pstr)
        {
            STRERROR("malloc");
        }
        *pstr->pstr = '\0';
    }
    return pstr;
}

/*@brief Internal funtion that calculates the actual value of a FROM_END index.*/
size_t __str_boundIndex(size_t ind, size_t clen)
{
    if (ind == STR_END) // It's after the last
    {
        return clen;
    }
    else if (ind & STR_END) // It's counted from the end
    {
        ind = (ind & ~STR_END);
        if (ind > clen)
        {
            STRFAIL("str indexing: Index (from-the-end) goes out of bounds from the left side.");
            return 0;
        }
        else 
        {
            return clen - ind;
        }
    }
    return ind;
}

// TODO: Maybe redesign this hot garbage function and change all int types to fixed width??? No? Ok. Fine.
/*@brief Returns a pointer to a new string populated with characters from the python-like slice [beg, ..., end) with a given step.
A negative step means that the order will be reversed. Use STR_END to indicate the item one after the last one. Use STR_FROMEND(n) to indicate the
n-th item from the back, ex. STR_FROMEND(1) is the last item (don't try STR_END-1 - it doesn't work!).
See also 'https://stackoverflow.com/questions/509211/how-slicing-in-python-works' for more information.
Invalid bounds that would cause the string to be empty, by default do not cause en error, unless DOOTSTR_SLICE_ERRORS is defined.
Otherwise, the function treats you like a baby and corrects your bounds to fit inside the string.
NOTE: values of beg/end cannot have the most significant bit, set to 1.*/
str_t *str_newslice(const dchar_t *cstring, size_t beg, size_t end, long step)
{
    if (!cstring)
    {
        STRFAIL("str_newslice: The passed address of the cstring is null.");
    }
    if (!(*cstring))
    {
        STR_SLICE_ERROR("str_newslice: The sliced cstring is empty.");
        return str_newfrom(STR_EMPTY);
    }
    if (step == 0)
    {
        STR_SLICE_ERROR("str_newsloice: The step cannot be zero. Resulting slice is empty.");
        return str_newfrom(STR_EMPTY);
    }

    size_t clen = _strlen(cstring);
    beg = __str_boundIndex(beg, clen); // In case the index is 'from the end'
    if (beg >= clen)
    {
        STR_SLICE_ERROR("str_newslice: Beg goes out of bounds from the right side.");
        beg = clen - 1;
    }
    end = __str_boundIndex(end, clen); // In case the index is 'from the end'
    if (end > clen)
    {
        STR_SLICE_ERROR("str_newslice: End goes out of bounds from the right side.");
        end = clen;
    }
    if (end < beg)
    {
        STR_SLICE_ERROR("str_newslice: End is less than beg - resulting slice is invalid (empty).");
        return str_newfrom(STR_EMPTY);
    }
    else if (end == beg)
    {
        STR_SLICE_ERROR("str_newslice: End is equal to beg - resulting slice is empty.");
        return str_newfrom(STR_EMPTY);
    }

    size_t sliceLen = 1 + (end - beg - 1) / labs(step); // This is correct I think
    str_t *slice = str_new(sliceLen + 1);
    size_t ind = (step > 0) ? beg : end - 1;
    size_t added = 0;
    //printf("Beg: %ld, end: %ld, step: %ld\n", beg, end, step);
    //printf("Slicelen: %ld\n", sliceLen);
    //printf("capacity: %ld\n", slice->capacity);
    while (added < sliceLen)
    {
        //printf("Ind: %ld\n", ind);
        slice->pstr[added++] = cstring[ind];
        if (step > 0)
        {
            ind += step;
        }
        else
        {
            ind += (size_t)(-step);
        }
    }
    slice->strlen = sliceLen;
    slice->pstr[slice->strlen] = '\0';
    return slice;
}

/*@brief Create a new doostr by taking ownership of an existing heap allocated c-style string. Other means of construction are
preferable, this is only for unique occasions.*/
str_t *str_steal(dchar_t *cstring)
{
    str_t *pstr = (str_t*)malloc(sizeof(str_t));
    if (!pstr)
    {
        STRERROR("malloc");
    }
    pstr->pstr = cstring;
    pstr->strlen = _strlen(cstring);
    pstr->capacity = pstr->strlen + 1;
    return pstr;
}

/*@brief Safely free a str_t object by passing the address of a pointer variable. The pointer will be set to null afterwards.*/
void str_free(str_t **ppstr)
{
    if (!ppstr)
    {
        STRFAIL("str_free: The address of a str_t pointer variable was null. Remember to pass an address (&variable) of a pointer variable to this function.");
    }
    if (!*ppstr)
    {
        return;
    }
    if ((*ppstr)->pstr)
    {
        free((*ppstr)->pstr);
    }
    free(*ppstr);
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

void str_assign_c(str_t *pstr, const dchar_t *cstring)
{
    if (!cstring)
    {
        STRFAIL("str_assign_c: Cannot assign to null char pointer.");
    }
    if (!pstr)
    {
        STRFAIL("str_assign_c: The address of a str_t was null.");
    }
    if (pstr->pstr == cstring)
    {
        return;
    }
    size_t clen = _strlen(cstring);
    if (!pstr->pstr)
    {
        str_realloc(pstr, clen + 1);
    }
    else if (pstr->capacity < clen + 1)
    {
        str_realloc(pstr, clen + 1);
    }
    pstr->strlen = clen;
    memcpy(pstr->pstr, cstring, (clen + 1) * sizeof(dchar_t));
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
    if (!pleft->pstr)
    {
        str_realloc(pleft, pright->strlen);
    }
    else if (pleft->capacity < pright->strlen + 1)
    {
        str_realloc(pleft, pright->strlen + 1);
    }
    pleft->strlen = pright->strlen;
    memcpy(pleft->pstr, pright->pstr, (pright->strlen + 1) * sizeof(dchar_t));
}

/*This function, unlike str_newslice by default allows you to use empty slices by setting beg the same as end.*/
void str_assignSlice(str_t *pstr, const dchar_t *cstring, size_t beg, size_t end, long step)
{
    if (!cstring)
    {
        STRFAIL("str_assignSlice: The passed address of the cstring is null.");
    }
    if (!(*cstring))
    {
        STR_SLICE_ERROR("str_assignSlice: The sliced cstring is empty.");
        str_assign_c(pstr, STR_EMPTY);
        return;
    }
    if (step == 0)
    {
        STR_SLICE_ERROR("str_assignSlice: The step cannot be zero. Resulting slice is empty.");
        str_assign_c(pstr, STR_EMPTY);
        return;
    }

    size_t clen = _strlen(cstring);
    beg = __str_boundIndex(beg, clen); // In case the index is 'from the end'
    if (beg > clen)
    {
        STR_SLICE_ERROR("str_assignSlice: Beg goes out of bounds from the right side.");
        beg = clen - 1;
    }
    end = __str_boundIndex(end, clen); // In case the index is 'from the end'
    if (end > clen)
    {
        STR_SLICE_ERROR("str_assignSlice: End goes out of bounds from the right side.");
        end = clen;
    }
    if (end < beg)
    {
        STR_SLICE_ERROR("str_assignSlice: End is less than beg - resulting slice is invalid (empty).");
        str_assign_c(pstr, STR_EMPTY);
        return;
    }
    else if (end == beg)
    {
        str_assign_c(pstr, STR_EMPTY);
        return;
    }
    size_t sliceLen = 1 + (end - beg - 1) / labs(step); // This is correct I think
    if (pstr->capacity < sliceLen + 1)
    {
        str_realloc(pstr, sliceLen + 1);
    }
    size_t ind = (step > 0) ? beg : end - 1;
    size_t added = 0;
    while (added < sliceLen)
    {
        pstr->pstr[added++] = cstring[ind];
        if (step > 0)
        {
            ind += step;
        }
        else
        {
            ind += (size_t)(-step);
        }

    }
    pstr->strlen = sliceLen;
    pstr->pstr[pstr->strlen] = '\0';
}

void str_append_c(str_t *pstr, const dchar_t *cstring)
{
    if (!pstr || !cstring)
    {
        STRFAIL("str_append: The address of a str_t or a c string was null.");
    }
    size_t rlen = _strlen(cstring);
    if (!pstr->pstr)
    {
        str_realloc(pstr, STR_NEWCAPACITY(rlen + 1));
        pstr->strlen = 0; // It should be zero already, just making sure tho
        if (!pstr->pstr)
        {
            STRERROR("realloc");
        }
    }
    else if (pstr->capacity < pstr->strlen + rlen + 1)
    {
        str_realloc(pstr, STR_NEWCAPACITY(pstr->strlen + rlen + 1));
        if (!pstr->pstr)
        {
            STRERROR("realloc");
        }
    }

    memcpy(pstr->pstr + pstr->strlen, cstring, (rlen + 1) * sizeof(dchar_t));
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
            STRERROR("realloc");
        }
    }
    else if (pleft->capacity < pleft->strlen + pright->strlen + 1)
    {
        str_realloc(pleft, STR_NEWCAPACITY(pleft->strlen + pright->strlen + 1));
        if (!pleft->pstr)
        {
            STRERROR("realloc");
        }
    }
    memcpy(pleft->pstr + pleft->strlen, pright->pstr, (pright->strlen + 1) * sizeof(dchar_t));
    pleft->strlen = pleft->strlen + pright->strlen;
}

/*
@brief Inserts a cstring starting at a given position in the str_t object. If the str_t is empty only position 0 is valid.
*/
void str_insert_c(str_t *pstr, const dchar_t *cstring, size_t position)
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
    size_t rlen = _strlen(cstring); 
    if ((!pstr->pstr || pstr->strlen == 0) && position != 0)
    {
        STRFAIL("str_insert_c: Cannot insert at a non zero position to an empty string.");
    }
    // Block is empty - allocating new block //
    if (!pstr->pstr)
    {
        str_realloc(pstr, STR_NEWCAPACITY(rlen + 1));
        memcpy(pstr->pstr, cstring, (rlen + 1) * sizeof(dchar_t));
        pstr->strlen = rlen;
        return;
    }
    // Block is too small, allocating new block and manually moving //
    if (pstr->capacity < pstr->strlen + rlen + 1)
    {
        size_t newcap = STR_NEWCAPACITY(pstr->strlen + rlen + 1);
        STR_LOG_ALLOC(pstr->capacity, newcap);
        dchar_t *newblock = (dchar_t *)malloc(sizeof(dchar_t) * newcap);  
        if (!newblock)
        {
            STRERROR("malloc");
        }
        memcpy(newblock, pstr->pstr, position * sizeof(dchar_t));
        memcpy(newblock + position, cstring, rlen * sizeof(dchar_t));
        memcpy(newblock + position + rlen, pstr->pstr + position, (pstr->strlen - position) * sizeof(dchar_t));
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
    memcpy(pstr->pstr + position, cstring, rlen * sizeof(dchar_t));
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
        memcpy(pleft->pstr, pright->pstr, (pright->strlen + 1) * sizeof(dchar_t));
        pleft->strlen = pright->strlen;
        return;
    }
    // Block is too small, allocating new block and manually moving //
    if (pleft->capacity < pleft->strlen + pright->strlen + 1)
    {
        size_t newcap = STR_NEWCAPACITY(pleft->strlen + pright->strlen + 1);
        STR_LOG_ALLOC(pleft->capacity, newcap);
        dchar_t *newblock = (dchar_t *)malloc(sizeof(dchar_t) * newcap);  
        if (!newblock)
        {
            STRERROR("malloc");
        }
        memcpy(newblock, pleft->pstr, position * sizeof(dchar_t));
        memcpy(newblock + position, pright->pstr, pright->strlen * sizeof(dchar_t));
        memcpy(newblock + position + pright->strlen, pleft->pstr + position, (pleft->strlen - position) * sizeof(dchar_t));
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
    memcpy(pleft->pstr + position, pright->pstr, pright->strlen * sizeof(dchar_t));
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
        memcpy(pstr->pstr, pleft->pstr, pleft->strlen * sizeof(dchar_t)); 
    }
    if (pright->pstr)
    {
        memcpy(pstr->pstr + pleft->strlen, pright->pstr, pright->strlen * sizeof(dchar_t));
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

size_t str_count(str_t *pstr, const dchar_t * seq); // Temporary solution to solve compilation issues.
size_t str_countAny(str_t *pstr, const dchar_t * set);

/*@brief Removes all occurances of seq from the string. Returns number of removed instances.*/
size_t str_remove(str_t *pstr, const dchar_t *seq)
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
    size_t rlen = _strlen(seq);
    size_t newLen = pstr->strlen - count *rlen;
    size_t *seqPos = (size_t*)malloc(sizeof(size_t)*count); // Array housing the positions of found substrings
    if (!seqPos)
    {
        STRERROR("malloc");
    }
    size_t i = 0;
    dchar_t *p = pstr->pstr;
    while ((p = _strstr(p, seq)) != NULL)
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
size_t str_removeAny(str_t *pstr, const dchar_t *set)
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
        STRERROR("malloc");
    }
    size_t i = 0;
    dchar_t *p = pstr->pstr;
    while ((p = _strpbrk(p, set)) != NULL)
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
    dchar_t *p = pstr->pstr;
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
    dchar_t *p = pstr->pstr;
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
    dchar_t *p = pstr->pstr;
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
    dchar_t *p = pstr->pstr;
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
    dchar_t *p = pstr->pstr;
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
    dchar_t *p = pstr->pstr;
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
int str_containsOnly(str_t *pstr, const dchar_t *set)
{
    if (!pstr)
    {
        STRFAIL("str_containsOnly: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    dchar_t *p = pstr->pstr;
    while (*p)
    {
        if (_strpbrk(p, set) != p)
        {
            return 0;
        }
        ++p;
    }
    return 1;
}

/*Returns 1 if the string contains any of the characters characters from a given set.
The function is just a wrapper for _strpbrk(), but I kept it for convenience.*/
int str_containsAny(str_t *pstr, const dchar_t *set)
{
    if (!pstr)
    {
        STRFAIL("str_containsAny: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    return _strpbrk(pstr->pstr, set) != NULL;
}

/*Returns 1 if the string contains seq as a substr.
The function is just a wrapper for _strstr() but I kept it for convenience.*/
int str_containsSeq(str_t *pstr, const dchar_t *seq)
{
    if (!pstr)
    {
        STRFAIL("str_containsAny: The address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        return 0;
    }
    return _strstr(pstr->pstr, seq) != NULL;
}
#pragma endregion

#pragma region TRANSFORM
void str_upper(str_t *pstr)
{
    if (!pstr)
    {
        STRFAIL("str_upper: The passed address was null.");
    }
    dchar_t *p = pstr->pstr;
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
    dchar_t *p = pstr->pstr;
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
    dchar_t *p = pstr->pstr;
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
size_t str_count(str_t *pstr, const dchar_t * seq)
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
    dchar_t *p = pstr->pstr;
    while ((p = _strstr(p, seq)) != NULL)
    {
        count++;
        p++;
    }
    return count;
}

/*@brief Counts how many times a character from a set is found in the string.*/
size_t str_countAny(str_t *pstr, const dchar_t * set)
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
    dchar_t *p = pstr->pstr;
    while ((p = _strpbrk(p, set)) != NULL)
    {
        count++;
        p++;
    }
    return count;
}

/*@brief Replaces each full occurance of oldval with newval. Returns the number of replaced instances. Always causes reallocation.
NOTE: This *can* be used for removing substrings, but isn't recomended as this function causes an unnecessary reallocation. In such cases
use str_remove() instead!*/
size_t str_replace(str_t *pstr, const dchar_t *oldval, const dchar_t *newval)  // I suck at C, debugging this was hell...
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
    size_t rlen = _strlen(newval), llen = _strlen(oldval);
    size_t newLen = (rlen > llen) ? pstr->strlen + count*(rlen-llen) : pstr->strlen - count*(llen-rlen);
    size_t *offsets = (size_t*)malloc(sizeof(size_t)*count);
    if (!offsets)
    {
        STRERROR("malloc");
    }
    size_t i = 0;
    dchar_t *p = pstr->pstr;
    while ((p = _strstr(p, oldval)) != NULL)
    {
        offsets[i++] = p - pstr->pstr;
        p += llen;
    }

    dchar_t *newblock;
    size_t blocksize;
    if (pstr->capacity < newLen + 1)
    {
        blocksize = newLen + 1;
    }
    else
    {
        blocksize = pstr->capacity;
    }
    newblock = (dchar_t *)malloc(sizeof(dchar_t) * blocksize);
    memset(newblock, 0, blocksize);
    STR_LOG_ALLOC(pstr->capacity, blocksize);
    if (!newblock)
    {
        STRERROR("malloc");
    }

    i = 0;
    size_t indOff = 0;
    size_t oldpos = 0;
    while (i < newLen + 1) // Plus 1 to copy '/0'
    {
        if (indOff < count && oldpos == offsets[indOff])
        {
            memcpy(newblock + i, newval, rlen * sizeof(dchar_t));
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
size_t str_replaceAny(str_t *pstr, const dchar_t *set, const dchar_t *newval)
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
    size_t rlen = _strlen(newval);
    size_t extraChars = count * rlen - count; // Additional needed characters
    size_t *offsets = (size_t*)malloc(sizeof(size_t)*count);
    if (!offsets)
    {
        STRERROR("malloc");
    }
    size_t i = 0;
    dchar_t *p = pstr->pstr;
    while ((p = _strpbrk(p, set)) != NULL)
    {
        offsets[i++] = p - pstr->pstr;
        p++;
    }

    dchar_t *newblock;
    size_t blocksize;
    if (pstr->capacity < pstr->strlen + extraChars + 1)
    {
        blocksize = pstr->strlen + extraChars + 1;
    }
    else
    {
        blocksize = pstr->capacity;
    }
    newblock = (dchar_t *)malloc(sizeof(dchar_t) * blocksize);
    STR_LOG_ALLOC(pstr->capacity, blocksize);
    if (!newblock)
    {
        STRERROR("malloc");
    }

    i = 0;
    size_t indOff = 0;
    size_t oldpos = 0;
    while (i < pstr->strlen + extraChars + 1) // Plus 1 to copy '/0'
    {
        if (indOff < count && oldpos == offsets[indOff])
        {
            memcpy(newblock + i, newval, rlen * sizeof(dchar_t));
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
size_t str_replaceAnyCh(str_t *pstr, const dchar_t *set, char c)
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
    dchar_t *p = pstr->pstr;
    while ((p = _strpbrk(p, set)) != NULL)
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
    dchar_t *p = pstr->pstr;
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
    dchar_t *p = pstr->pstr;
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
    dchar_t *p = pstr->pstr + pstr->strlen - 1;
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
ssize_t str_index(str_t *pstr, const dchar_t *seq)
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
    const dchar_t *ps = seq;
    // This could be _strstr(pstr->pstr) - pstr->pstr; But this is more easily modifiable.
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
ssize_t str_rindex(str_t *pstr, const dchar_t *seq)
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
    ssize_t rlen = _strlen(seq);
    const dchar_t *ps = seq + rlen - 1;
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
The passed output strings can but don't have to be initialized.
@param[in] pstr input string (non empty)
@param[in] pivot pivot sequence
@param[in] pivot str_t containing the substring before the pivot.
@param[in] outleft str_t containing the substring before the pivot.
@param[in] outmid str_t containing the substring before the pivot.*/
void str_partition(str_t *pstr, const dchar_t *pivot, str_t **outleft, str_t **outmid, str_t **outright)
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
    if (!*outleft)
    {
        *outleft = str_new(0);
    }
    if (!*outmid)
    {
        *outmid = str_new(0);
    }
    if (!*outright)
    {
        *outright = str_new(0);
    }
    ssize_t pivInd = str_index(pstr, pivot);
    size_t pivLen = _strlen(pivot);
    if (pivInd == -1)
    {
        str_assign_c(*outright, STR_EMPTY);
        str_assign_c(*outmid, pivot);
        str_assign_c(*outright, STR_EMPTY);
    }
    else
    {
        str_assignSlice(*outleft, pstr->pstr, 0, pivInd, 1);
        str_assign_c(*outmid, pivot);
        str_assignSlice(*outright, pstr->pstr, pivInd + pivLen, pstr->strlen, 1);
    }
}

void str_rpartition(str_t *pstr, const dchar_t *pivot, str_t **outleft, str_t **outmid, str_t **outright)
{
    if (!pstr)
    {
        STRFAIL("str_rpartition: The passed address of a str_t was null.");
    }
    if (!pstr->pstr)
    {
        STRFAIL("str_rpartition: The passed string was empty.");
    }
    if (!pivot)
    {
        STRFAIL("str_rpartition: The passed address of pivot was null.");
    }
    if (!(*pivot))
    {
        STRFAIL("str_rpartition: The passed pivot was empty.");
    }
    if (!outleft || !outmid || !outright)
    {
        STRFAIL("str_rpartition: One of the passed addreses of output str_t pointer variables was null.");
    }
    if (!*outleft)
    {
        *outleft = str_new(0);
    }
    if (!*outmid)
    {
        *outmid = str_new(0);
    }
    if (!*outright)
    {
        *outright = str_new(0);
    }
    ssize_t pivInd = str_rindex(pstr, pivot);
    size_t pivLen = _strlen(pivot);
    if (pivInd == -1)
    {
        str_assign_c(*outright, STR_EMPTY);
        str_assign_c(*outmid, pivot);
        str_assign_c(*outright, STR_EMPTY);
    }
    else
    {
        str_assignSlice(*outleft, pstr->pstr, 0, pivInd, 1);
        str_assign_c(*outmid, pivot);
        str_assignSlice(*outright, pstr->pstr, pivInd + pivLen, pstr->strlen, 1);
    }
}

/*
@brief This struct represents an array of str_t objects. It's used by some dootstr functions, to make dealing with many strings easier
and safer. Now ideally this would be a dynamic vector, but at this point just use C++. This struct is mostly meant to be recieved by the user from
str_ functions, not necessarily created.
*/
typedef struct sarr
{
    str_t **strArr; /*The array of str_t pointers (to single str_t structs).*/
    size_t size; /*The number of strings in the array.*/
} sarr_t;

/*@brief Create a sarr_t by taking onwership of an existing array of c-style strings.
The cstring pointer is freed, dangling and unusable afterwards. Make sure that the provided array is of size n,
otherwise excpect read violations. Please be careful with this.*/
sarr_t *str_asteal(dchar_t **cstrings, size_t n)
{
    if (!cstrings)
    {
        STRFAIL("str_asteal: The passed address to cstrings is null.");
    }
    for (size_t i = 0; i < n; i++)
    {
        if (!(cstrings[i]))
        {
            STRFAIL("str_asteal: One of the strings in the cstrings array is null");
        }
    }

    sarr_t *parr = (sarr_t*)malloc(sizeof(sarr_t));
    if (!parr)
    {
        STRERROR("malloc");
    }
    parr->strArr = (str_t**)malloc(sizeof(str_t*) * n);
    if (!parr->strArr)
    {
        STRERROR("malloc");
    }
    parr->size = n;
    for (size_t i = 0; i < n; i++)
    {
        parr->strArr[i] = str_steal(cstrings[i]);
    }
    free(cstrings);
    return parr;
}

/*@brief Create a sarr_t by copying an already existing array of c-style strings.*/
sarr_t *str_afrom(dchar_t **cstrings, size_t n)
{
    if (!cstrings)
    {
        STRFAIL("str_afrom: The passed address to cstrings is null.");
    }
    for (size_t i = 0; i < n; i++)
    {
        if (!(cstrings[i]))
        {
            STRFAIL("str_afrom: One of the strings in the cstrings array is null");
        }
    }

    sarr_t *parr = (sarr_t*)malloc(sizeof(sarr_t));
    if (!parr)
    {
        STRERROR("malloc");
    }
    parr->strArr = (str_t**)malloc(sizeof(str_t*) * n);
    if (!parr->strArr)
    {
        STRERROR("malloc");
    }
    parr->size = n;
    for (size_t i = 0; i < n; i++)
    {
        parr->strArr[i] = str_newfrom(cstrings[i]);
    }
    return parr;
}

/*Safely free a sarr_t by passing the addres of a str_t* variable. The variable will be set to NULL afterwards.*/
void str_afree(sarr_t **pparr)
{
    if (!pparr)
    {
        STRFAIL("str_afree: The passed address of a sarr_t* variable is null.");
    }
    if (!*pparr)
    {
        return;
    }
    if (!(*pparr)->strArr)
    {
        (*pparr)->size = 0;
        free(*pparr);
        *pparr = NULL;
        return;
    }
    for (size_t i = 0; i < (*pparr)->size; i++)
    {
        str_free((*pparr)->strArr + i);
    }
    free((*pparr)->strArr);
    (*pparr)->strArr = NULL;
    (*pparr)->size = 0;
    free(*pparr);
    *pparr = NULL;
}

/*@brief Internal function that counts how many parts spliting the string by delim will result in.
I have a feeling this is horribly written and could be rewritten to 1/2 the length. This is godawful, but works.*/
size_t __str_countSplits(str_t *pstr, const dchar_t *delim, size_t *dsize)
{
    if (!pstr)
    {
        STRFAIL("str_split: The passed address of str_t was null.");
    }
    if (pstr->strlen == 0)
    {
        return 0;
    }
    if (!delim)
    {
        STRFAIL("str_split: The passed delim is null.");
    }
    if (!*delim)
    {
        return 1;
    }
    size_t dlen = _strlen(delim);
    if (dsize)
    {
        *dsize = dlen;
    }
    const dchar_t *p = pstr->pstr, *plast = p;
    size_t count = 0;
    short anyDelim = 0;
    while ((p = _strstr(p, delim)) != NULL)
    {
        // Check if character to the left is not outside or part of another delim sequence
        if (p == pstr->pstr || (plast + dlen >= p && anyDelim))
        {
            anyDelim = 1;
            plast = p;
            p += dlen;
            continue;
        }
        anyDelim = 1;
        count++;
        plast = p;
        p += dlen;
    }
    if (plast == pstr->pstr)
    {
        //There was either no delim or a single delim at the beginning or 
        if (anyDelim)
        {
            return pstr->strlen > dlen;
        }
        return 1;
    }
    if (plast + dlen - pstr->pstr < pstr->strlen)
    {
        count ++; // Count the last one
    }
    return count;
}

/*@brief Splits the string by delim and returns an array (sarr_t) of resulting strings.*/
sarr_t *str_split(str_t *pstr, const dchar_t *delim)
{
    size_t dlen;
    size_t numSplits = __str_countSplits(pstr, delim, &dlen);
    sarr_t *parr = (sarr_t*)malloc(sizeof(sarr_t));
    if (!parr)
    {
        STRERROR("malloc");
    }
    parr->size = numSplits;
    if (parr->size == 0)
    {
        parr->strArr = NULL;
        return parr;
    }

    parr->strArr = (str_t**)malloc(sizeof(str_t*)*parr->size);
    if (!parr->strArr)
    {
        STRERROR("malloc");
    }
    if (parr->size == 1)
    {
        parr->strArr[0] = str_newfrom(pstr->pstr);
        return parr;
    }
    parr->strArr = (str_t**)malloc(sizeof(str_t*)*parr->size);
    if (!parr->strArr)
    {
        STRERROR("malloc");
    }
    
    // size_t dlen = _strlen(dlen);
    // size_t ind = 0;
    // const dchar_t *p = pstr->pstr;
    // const dchar_t *plast = p;
    // while ((p = _strstr(p, delim)) != NULL)
    // {
    //     //parr->strArr[ind] = str_newslice(plast, dlen, p - plast);
    //     plast = p;
    //     p++;
    //     ind++;

    // }
    return NULL;
}

/*
splitlines()
join()
split => returns a string array with a size
*/

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

#pragma region VIEWING
/*Safely access the i-th string character with bound checking and from-the-end indexing support.*/
char str_at(str_t* pstr, size_t i)
{
    size_t boundInd = __str_boundIndex(i, pstr->strlen);
    if (boundInd > pstr->strlen)
    {
        STRFAIL("str_at: Index is out of bounds from the right side.");
    }
    return pstr->pstr[boundInd];
}
#pragma endregion

#pragma region FORMATTING
//Time to do this bitch... later
void str_format()
{

}
#pragma endregion

#endif
