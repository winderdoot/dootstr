
# dootstr

Simple C string library by widerdoot.  
I have no idea how to write markdown files, I'll fix this up as I go.  
There will be documentation here and shit. For now it's a to do list.  
See [dootguide](dootguide.md) to learn how to use this library. 

## Notes on allocations

The general rule for allocating memory, is that when a method that expands the string is called and needs more memory, it will
realocate to a size twice as big as the amount it needs in that moment. In that regard it's simillar to most popular dynamic vector implementations. The are exceptions to this - some methods that only somewhat modify the string and end up needing more memory, will only allocate the exact amount needed (ex. ```str_replace()``` and it's variants). Another exception is ```str_assign()``` and it's variants.
To directly control the amount of memory used, use ```str_new(size_t capacity)``` to create a string with a certain amount of memory preallocated. Alternatively directly use ```str_realloc(size_t newCapacity)``` to reallocate the string's memory to a new size. This can be used to trim the string and free up unused memory, however it depends on the standard ```realloc()``` function whether or not this will just shorten the block or cause a full reallocation. Use it wisely.  

## Issues I'm aware of

Guarding against huge allocations has been added.
Another issue is the usage of ssize_t for indexes that could be -1. It's Unix only and not part of the C standard. Too bad.
The C language doesn't include a signed integer type that is guaranteed to hold negative values of the same magnitude as size_t. Too bad.
I could also remove the need for allocating the ```pstr``` pointer in the ```str_t``` struct by storing the string data after the ```capacity``` and ```strlen``` fields, directly in the memory of the struct. That would reduce memory overhead, but it would make shortening and reallocating the string very cumbersome, as the address the user uses to access the string could need to change. I won't be implementing that.

## Unicode support

I've added barebones UNICODE support. If ```DOOTSTR_USE_WCHAR``` is defined, then ```wchar_t``` will be the char type used in the implementation. I haven't tested it on windows yet, because I still need to add fixed width integers into the implementation first. I'm way to sleepy rn. it can wait

## TODO`

Modify sarr_t to have the str_t array inplace with the sarr_t struct memory, that way caching is utilized.
The sarr_t arrays don't change size so they don't need to be reallocated.

standardize error messages

slice_view
slice_copy

tokenize => simillar to strtok
other python like methods (if any are missing)

format()

Probably not gonna implement stuff that would be indentical to an already existing C library function.
At most I might lightly wrap some of them.

C str functions that have macros to replace them with wchar_t versions if needed:
strdup => wcsdup
strstr => wcsstr
"" => L""
strpbrk => wcspbrk
