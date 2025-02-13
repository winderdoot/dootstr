
# dootstr

Simple C string library by widerdoot.  
I have no idea how to write markdown files, I'll fix this up as I go.  
There will be documentation here and shit. For now it's a to do list.  

## Notes on allocations

The general rule for allocating memory, is that when a method that expands the string is called and needs more memory, it will
realocate to a size twice as big as the amount it needs in that moment. In that regard it's simillar to most popular dynamic vector implementations. The are exceptions to this - some methods that only somewhat modify the string and end up needing more memory, will only allocate the exact amount needed (ex. ```str_replace()``` and it's variants).  
To directly control the amount of memory used, use ```str_new(size_t capacity)``` to create a string with a certain amount of memory preallocated. Alternatively directly use ```str_realloc(size_t newCapacity)``` to reallocate the string's memory to a new size. This can be used to trim the string and free up unused memory, however it depends on the standard ```realloc()``` function whether or not this will just shorten the block or cause a full reallocation. Use it wisely.  

## TODO

slice_view
slice_copy

split() rsplit()
partition() rpartition()
splitlines()
join()
other python like methods

index() => index_c, index

strip(), lstrip(), rstrip()
format()

Probably not gonna implement stuff that would be indentical to an already existing C library function.
At most I might lightly wrap some of them.
