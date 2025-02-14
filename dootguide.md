
# Dootstr tutorial

See [dootdocs](dootdocs.md) for full documentation (if I made it).  
See [README](README.md) for extras.

## str_t struct

The struct used in this library is as follows:

```C
typedef struct str
{
    char *pstr; /*Null terminated pointer to the char data*/
    size_t strlen; /*Number of stored readable characters*/
    size_t capacity; /*Current size of the allocated memory block*/

} str_t;
```

These fields do not consent to being modified, treat them as readonly. Feel the ```pstr``` pointer to C library functions, as long
as they don't modify it. ```strlen``` is usefull and it's updated as the string grows, same goes for ```capacity```.

## Creating a string

The recomended way to use a ```str_t``` is to allocate it on the heap and access it via a pointer:

```C
size_t *s = str_newfrom("Hello dootstr!");
puts(s->pstr);

> Hello dootstr!
```

If you know the rough size of the string ahead of time, but not the content, use:

```C
size_t capacity = 2137;
size_t *s = str_new(capacity);
```

The string will be empty, null terminated and ready for further use.