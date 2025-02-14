
# dootstr

Simple C string library by widerdoot.  
I have no idea how to write markdown files, I'll fix this up as I go.  
There will be documentation here and shit. For now it's a to do list.  
See [dootguide](dootguide.md) to learn how to use this library.

## Notes on allocations

The general rule for allocating memory, is that when a method that expands the string is called and needs more memory, it will
realocate to a size twice as big as the amount it needs in that moment. In that regard it's simillar to most popular dynamic vector implementations. The are exceptions to this - some methods that only somewhat modify the string and end up needing more memory, will only allocate the exact amount needed (ex. ```str_replace()``` and it's variants).  
To directly control the amount of memory used, use ```str_new(size_t capacity)``` to create a string with a certain amount of memory preallocated. Alternatively directly use ```str_realloc(size_t newCapacity)``` to reallocate the string's memory to a new size. This can be used to trim the string and free up unused memory, however it depends on the standard ```realloc()``` function whether or not this will just shorten the block or cause a full reallocation. Use it wisely.  

## Issues I'm aware of

Currently I'm not guarding against allocating too much memory. The capacity type of size_t is big, but not infinite. Too bad.
Another issue is the usage of ssize_t for indexes that could be -1. It's Unix only and not part of the C standard. Too bad.
The C language doesn't include a signed integer type that is guaranteed to hold negative values of the same magnitude as size_t. Too bad.

## TODO

A method for construction from a pointer + length (checking for out out bounds)
str_newslice - your lord and saviour!

slice_view
slice_copy

partition() rpartition()
splitlines()
join()
split => returns a string array with a size
tokenize => simillar to strtok
other python like methods

strip(), lstrip(), rstrip()
format()

Probably not gonna implement stuff that would be indentical to an already existing C library function.
At most I might lightly wrap some of them.
