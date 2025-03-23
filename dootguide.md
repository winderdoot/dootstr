
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

These fields do not consent to being modified, treat them as readonly. Feel free to pass the ```pstr``` pointer to C library functions, as long as they don't modify it. ```strlen``` is usefull and it's updated as the string grows, same goes for ```capacity```.

## Creating a string

The recomended way to use a ```str_t``` is to allocate it on the heap and access it via a pointer:

```C
size_t *s = str_newfrom("Hello dootstr!");
puts(s->pstr);

> Hello dootstr!
```

If you know the rough size of the string ahead of time, but not the content, use ```str_new()```:

```C
size_t capacity = 2137;
size_t *s = str_new(capacity);
```

The string will be empty, null terminated and ready for further use.

## Freeing a string

God intended for his children to manually manage all memory, so to safely free a ```str_t``` struct, use ```str_free()```:

```C
size_t *s = str_newfrom("Hello dootstr!");
str_free(&s);
```

You can always safely call this function on any ```size_t*``` variable.
Notice, that ```str_free``` requires a double pointer, so that it can set your pointer variable to ```NULL``` afterwards.

## Adding characters to the string

To assign characters to a string, use ```str_assign()```:

```C
str_t *s1 = str_new(0); // You could as well give it a size ahead of time
str_assign_c(s1, "I love SOP!");
str_t *s2 = str_newfrom("But I don't...");
str_assign(s2, s1);
```

Notice that many functions have ```_c``` variants that accept c-style strings as the second string. For small strings you don't need to modify, it's more convenient than having to specifically create a ```str_t```.

The best way to dinamically grow a string is to use ```str_append()```:

```C
str_t *msg = str_new(50);
str_append_c(msg, "Lorem");
str_append_c(msg, "Ipsum");
str_t *msg1 = str_new(50);
str_apend(msg1, msg);
str_append_c(msg1, "Dolor Sit Amet");
```

To insert a sequence inside a string, use ```str_insert(str_t *pstr, str_t *sequence, size_t position)```:

```C

str_t *s = str_newfrom("I like puppies!");
str_insert_c(s, "happy ", 6);
puts(s->pstr);
> I like happy pupies!
```

Position 6 targets the second space. Notice that ```"happy "``` is inserted between character 6 and 7 (counting from 0).

## Removing characters from the string

To remove a certain portion of the string, that is *cut it out*, use ```str_cut(str_t *pstr, size_t position, size_t length)```:

```C
str_t *s = str_newfrom("Javascript is a badass language");
str_cut(s, 20, 3);
puts(s->pstr);
> Javascript is a bad language
```

To remove any ocurance of a given substring use ```str_remove(str_t *pstr, const char *substr)```:

```C
str_t *s = str_newfrom("C++ is the best!");
str_remove(s, "++");
puts(s->pstr);
> C is the best!
```

To remove surrounding whitespace characters, use ```str_lstrip()```, ```str_strip()``` and ```str_rstrip()```

## Replacing characters

You can directly modify the character data of the string via the ```pstr``` struct field, but It's preferable to use library functions if possible. For replacing characters, there are a few functions available:

+ ```str_replace(str_t *pstr, const char *oldval, const char *newval)``` - replaces any found substring ```oldval``` with ```newval```
+ ```str_replaceAny(str_t *pstr, const char *set, const char *newval)``` - replaces any character also present in the string  ```set``` with the string ```newval```
+ ```str_replaceAnyCh(str_t *pstr, const char *set, char c)``` - replaces any character also present in the sting ```set``` with the character ```c```

## Logical functions

## Slicing and partitions

other stuff idk
