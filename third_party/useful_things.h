/*
This is an stb-style single header library providing simple quality of life utilities for C programming.

To use simply add these two lines once in your project:

#define USEFUL_THINGS_IMPLEMENTATION
#include "useful_things.h"

Documentation:

Memory management and arenas:
An arena is a linear allocator for objects with a shared lifetime. You create an arena, allocate as
many things as you want, then free the entire arena once. In practice the arena is implemented as a
linked list of separate allocations. When you attempt to allocate into the arena it will either find
space in the available allocations in the list or add a new allocation to the linked list. This way
the arena is dynamic, while still preserving all pointers to previous arena allocations, it will
never reallocate or move previously allocated data.

procedures:

UT_Arena *UT_arena_create_size(size_t size);
Create a new arena of at least the specified size.

UT_Arena *UT_arena_create();
Create a new arena of default size (64 kilobytes)

void *UT_arena_alloc(UT_Arena *arena, size_t num_objects, size_t object_size);
Allocate into an arena
Args:
    arena: the arena to allocate in
    num_objects: how many objects to allocate
    object_size: size of one object
Returns:
    a pointer to the allocated region

void UT_arena_free(UT_Arena *arena);
*/

// TODO: don't depend on most of these
#ifndef USEFUL_THINGS_H
#define USEFUL_THINGS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h> // Need FILE * for some functions as of right now

// common types
#define UT_KILOBYTES(n) ((1024) * (n))
#define UT_MEGABYTES(n) ((1024) * (KILOBYTES(n)))
#define UT_GIGABYTES(n) ((1024) * (MEGABYTES(n)))

#define UT_ARRAY_LENGTH(arr) (sizeof (arr) / sizeof (arr)[0])

// memory management and arenas
#define UT_ARENA_DEFAULT_SIZE UT_KILOBYTES(64)

typedef struct UT_Arena UT_Arena;
struct UT_Arena {
    struct UT_Arena *next;
    size_t size;
    uint8_t *cur;
    uint8_t memory[];
};

UT_Arena *UT_arena_create_size(size_t size);
UT_Arena *UT_arena_create();

void *UT_arena_alloc(UT_Arena *arena, size_t num_objects, size_t object_size);
void *UT_arena_pop(UT_Arena *arena, size_t bytes_to_pop);
void UT_arena_free(UT_Arena *arena);

// quick and dirty dynamic array implementation. Use by defining a type with at least these three members: T* data, int count, int capacity.
// you can also use the UT_da_decl(T) to declare it which defines a type TList.
#define UT_da_append(arr, item)                                                    \
do {                                                                               \
    if((arr)->count >= (arr)->capacity) {                                          \
        (arr)->capacity = (arr)->capacity == 0 ? 8 : (arr)->capacity*2;            \
        (arr)->data = realloc((arr)->data, (arr)->capacity * sizeof *(arr)->data); \
        assert((arr)->data && "Failed to realloc");                                \
    }                                                                              \
    (arr)->data[(arr)->count++] = (item);                                          \
} while(0)

/*
API:
    Strings
Overview:
    Length based strings in C. Functions that return strings fall into two
    categories: those that allocate new strings and those that don't. the two
    groups can be easily distinguished by whether or not the function takes a
    UT_Arena pointer as an argument. While this library does not require any
    UT_String to be null-terminated it will still null-terminate any newly
    allocated strings for interoperability with other apis. HOWEVER; some
    functions return strings that are not newly allocated, but are instead
    simply slices into a larger buffer, for simplicity and performance,
    therefor you cannot always assume to be able to pass a string from this
    library to an api that expects null-terminated strings. A few helper
    functions exist for this reason such as: UT_is_null_terminated(UT_String s)
    and UT_make_null_terminated(UT_String s, UT_Arena *arena) Any allocations
    are made in an arena passed to the function. Strings in this category are
    guaranteed to be null-terminated. Any strings returned from functions that
    do not take an arena pointer are not guaranteed to be null-terminated, as
    they are often simply slices into a larger string.
*/

typedef struct {
    char *data;
    size_t length;
} UT_String;

// for use with string literals, not char pointers. sizeof("String") gives the length of the string plus one for null terminator
#define UT_STR(s) (String){ .data = (s), .length = sizeof(s) - 1 }
UT_String UT_make_string(char *s, int length, UT_Arena *arena);
UT_String UT_copy_string(UT_String s, UT_Arena *arena);
UT_String UT_slice_to_string(char *p, int length);
UT_String UT_null_term_to_string(char *s);
UT_String UT_make_null_terminated(UT_String s, UT_Arena *arena);
UT_String UT_string_concat(UT_String s1, UT_String s2, UT_Arena *arena);
UT_String *UT_split_string(UT_String s, char delimiter, size_t *out_length, bool copy_substrings, UT_Arena *arena);
UT_String UT_to_lower(UT_String s, UT_Arena *arena);
UT_String UT_to_upper(UT_String s, UT_Arena *arena);
bool UT_strings_are_equal(UT_String s1, UT_String s2);
// this will read out-of-bounds memory in some cases lmao
bool UT_is_null_terminated(UT_String s);

// file management
size_t UT_get_file_size(FILE *f);
UT_String UT_read_entire_file_as_string(UT_String file_path, UT_Arena *arena);
char *UT_read_entire_file_and_null_terminate(char *file_path);
char *UT_read_entire_file_and_null_terminate_arena(char *file_path, UT_Arena *arena);

char *UT_read_entire_file(char *file_path);
UT_String *UT_list_directory(char *dir_path, size_t *out_length, UT_Arena *arena);

#endif

// NOTE: This section lives outside the header include guards so that the prefixes can be stripped at any point, even if the translation unit has already included the file before
#ifdef USEFUL_THINGS_STRIP_PREFIX

#define KILOBYTES UT_KILOBYTES
#define MEGABYTES UT_MEGABYTES
#define GIGABYTES UT_GIGABYTES

#define ARRAY_LENGTH UT_ARRAY_LENGTH

#define Arena UT_Arena
#define arena_create_size UT_arena_create_size
#define arena_create UT_arena_create
#define arena_alloc UT_arena_alloc
#define arena_free UT_arena_free

#define da_append UT_da_append

#define String UT_String
#define STR UT_STR
#define make_string UT_make_string
#define copy_string UT_copy_string
#define slice_to_string UT_slice_to_string
#define null_term_to_string UT_null_term_to_string
#define make_null_terminated UT_make_null_terminated
#define string_concat UT_string_concat
#define split_string UT_split_string
#define strings_are_equal UT_strings_are_equal
#define is_null_terminated UT_is_null_terminated
#define to_lower UT_to_lower
#define to_upper UT_to_upper

#define read_entire_file_as_string UT_read_entire_file_as_string
#define list_directory UT_list_directory

#endif

#ifdef USEFUL_THINGS_IMPLEMENTATION

#include <assert.h>
#include <stddef.h> // ptrdiff_t
#include <dirent.h>
#include <string.h> // strlen
#include <stdlib.h>

#define UT_GET_MEMORY(size) malloc(size)
#define UT_FREE_MEMORY(mem_pointer) free(mem_pointer)
#define UT_MEMCPY(dst, src, len) memcpy(dst, src, len)

// Memory management and arenas
UT_Arena *UT_arena_create_size(size_t size) {
    UT_Arena *arena = UT_GET_MEMORY(sizeof(*arena) + size);
    if(!arena) return NULL;
    arena->next = NULL;
    arena->size = size;
    arena->cur = arena->memory;
    return arena;
}

UT_Arena *UT_arena_create() {
    uint32_t default_size = UT_ARENA_DEFAULT_SIZE;
    return UT_arena_create_size(default_size);
}

void *UT_arena_alloc(UT_Arena *arena, size_t bytes_to_allocate, size_t alignment) {
    UT_Arena *linked_arena = arena;
    // Loop through all the arenas in the linked list. If one has space, allocate there and return.
    while(linked_arena) {
        size_t space_left = (linked_arena->memory + linked_arena->size) - linked_arena->cur;
        // handle misalignment
        if((ptrdiff_t)linked_arena->cur % alignment != 0) {
            uint32_t bytes_to_add_for_alignment = alignment - ((ptrdiff_t)linked_arena->cur % alignment);
            space_left -= bytes_to_add_for_alignment;
            if(bytes_to_allocate <= space_left)
                linked_arena->cur += bytes_to_add_for_alignment;
        }
        if(bytes_to_allocate <= space_left) {
            void *ret = linked_arena->cur;
            linked_arena->cur += bytes_to_allocate;
            assert((ptrdiff_t)ret % alignment == 0 && "Misaligned allocation");
            return ret;
        }
        else {
            linked_arena = linked_arena->next;
        }
    }
    // No arena had space. Create a new one and add to linked list
    // oversize it slightly to handle alignment
    size_t new_arena_size = bytes_to_allocate + alignment > arena->size ? bytes_to_allocate + alignment : arena->size;
    UT_Arena *new_arena = UT_arena_create_size(new_arena_size);

    new_arena->next = arena->next;
    arena->next = new_arena;
    // handle misalignment
    if((ptrdiff_t)new_arena->cur % alignment != 0) {
        uint32_t bytes_to_add_for_alignment = alignment - ((ptrdiff_t)new_arena->cur % alignment);
        new_arena->cur += bytes_to_add_for_alignment;
    }
    void *ret = new_arena->cur;
    assert((ptrdiff_t)ret % alignment == 0 && "Misaligned allocation");
    new_arena->cur += bytes_to_allocate;
    return ret;
}

void *UT_arena_pop(UT_Arena *arena, size_t bytes_to_pop) {
    // this is complicated for my small brain with the linked arenas, but this should work ok in most cases??
    // I guess this can actually cause some major memory bugs so definitely should fix this TODO
    if(arena->next) arena = arena->next;
    arena->cur = bytes_to_pop > (size_t)(arena->cur - arena->memory) ? 0 : arena->cur - bytes_to_pop;
    return arena->cur;
}


void UT_arena_free(UT_Arena *arena) {
    UT_Arena *next = arena;
    while(next->next) {
        arena = next;
        next = next->next;
        UT_FREE_MEMORY(arena);
    }
    UT_FREE_MEMORY(next);
}

// strings
// TODO: what is a good name for this? Should have a clear naming convention for this kind of thing
UT_String UT_make_string(char *s, int length, UT_Arena *arena) {
    char *data = UT_arena_alloc(arena, length + 1, 1);
    UT_MEMCPY(data, s, length);
    data[length] = 0;
    return (UT_String){ .data = data, .length = length };
}

UT_String UT_slice_to_string(char *p, int length) {
    return (UT_String){ .data = p, .length = length };
}

UT_String UT_null_term_to_string(char *s) {
    int length = 0;
    while(*(s + length)) {
        ++length;
    }
    return UT_slice_to_string(s, length);
}

UT_String UT_copy_string(UT_String s, UT_Arena *arena) {
    return UT_make_string(s.data, s.length, arena);
}

UT_String UT_make_null_terminated(UT_String s, UT_Arena *arena) {
    if(UT_is_null_terminated(s)) return s;
    return UT_copy_string(s, arena);
}

UT_String UT_string_concat(UT_String s1, UT_String s2, UT_Arena *arena) {
    char *data = UT_arena_alloc(arena, s1.length + s2.length + 1, 1);
    UT_MEMCPY(data, s1.data , s1.length);
    UT_MEMCPY(data + s1.length, s2.data , s2.length);
    data[s1.length + s2.length] = 0;
    return (UT_String){ .data = data, .length = s1.length + s2.length };
}

//UT_String UT_string_concat_many(UT_String *strings, size_t strings_length, UT_Arena *arena) {
//    UT_String ret = {0};
//    ret.length = 0;
//    for(int i = 0; i < strings_length; ++i) {
//        ret.length += strings[i].length;
//    }
//    ret.data = UT_arena_alloc(arena, ret.length + 1, 1);
//    char *cur = ret.data;
//    for(int i = 0; i < strings_length; ++i) {
//     //TODO: not implemented
//                                                           
//    }
//}

bool UT_strings_are_equal(UT_String s1, UT_String s2) {
    if(s1.length != s2.length) return false;
    for(size_t i = 0; i < s1.length; ++i) {
        if(s1.data[i] != s2.data[i]) return false;
    }
    return true;
}

bool UT_is_null_terminated(UT_String s) {
    return s.data[s.length] == 0;
}

UT_String *UT_split_string(UT_String s, char delimiter, size_t *out_length, bool copy_substrings, UT_Arena *arena) {
    *out_length = 0;
    // determine how many splits
    {
        int substring_len = 0;
        for(size_t i = 0; i < s.length; ++i) {
            if(s.data[i] == delimiter && substring_len > 0) {
                ++(*out_length);
                substring_len = 0;
            }
            else {
                ++substring_len;
            }
        }
        if(substring_len > 0) {
            ++(*out_length);
        }
    }
    UT_String *substrings = UT_arena_alloc(arena, *out_length * sizeof(*substrings), sizeof(*substrings));
    {
        int substring_idx = 0;
        int substring_len = 0;
        for(size_t i = 0; i < s.length; ++i) {
            if(s.data[i] == delimiter) {
                if(substring_len > 0) {
                    substrings[substring_idx++] = copy_substrings ?
                        UT_make_string(&(s.data[i - substring_len]), substring_len, arena) :
                        UT_slice_to_string(&(s.data[i - substring_len]), substring_len);
                    substring_len = 0;
                }
            } else {
                ++substring_len;
            }
        }
        if(substring_len > 0) {
            substrings[substring_idx] = copy_substrings ?
                UT_make_string(&(s.data[s.length - substring_len]), substring_len, arena) :
                UT_slice_to_string(&(s.data[s.length - substring_len]), substring_len);
        }
    }
    return substrings;
}

UT_String UT_to_lower(UT_String s, UT_Arena *arena) {
    UT_String lower_case = UT_copy_string(s, arena);
    for(size_t i = 0; i < lower_case.length; ++i) {
        if(lower_case.data[i] >= 'A' && lower_case.data[i] <= 'Z') {
            lower_case.data[i] += 'a' - 'A';
        }
    }
    return lower_case;
}

UT_String UT_to_upper(UT_String s, UT_Arena *arena) {
    UT_String upper_case = UT_copy_string(s, arena);
    for(size_t i = 0; i < upper_case.length; ++i) {
        if(upper_case.data[i] >= 'a' && upper_case.data[i] <= 'z') {
            upper_case.data[i] -= 'a' - 'A';
        }
    }
    return upper_case;
}

// File system handling
size_t UT_get_file_size(FILE *f) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

// reads the file contents directly into a malloced buffer and nothing else.
// to read text files into c-strings it's better to use UT_read_entire_file_and_null_terminate()
char *UT_read_entire_file(char *file_path) {
    FILE *f = NULL;
    char *buf = NULL;

    f = fopen(file_path, "r");
    if(!f) {
        perror(NULL);
        goto fail;
    }

    size_t file_size = UT_get_file_size(f);
    buf = (char *)UT_GET_MEMORY(file_size);
    size_t items_read = fread(buf, file_size, 1, f);
    if(!items_read) {
        goto fail;
    }

    fclose(f);

    return buf;

fail:
    if(f) fclose(f);
    if(buf) UT_FREE_MEMORY(buf);
    return NULL;
}

char *UT_read_entire_file_and_null_terminate(char *file_path) {
    FILE *f = NULL;
    char *buf = NULL;

    f = fopen(file_path, "r");
    if(!f) {
        perror(NULL);
        goto fail;
    }

    size_t file_size = UT_get_file_size(f);
    buf = (char *)UT_GET_MEMORY(file_size + 1);
    size_t items_read = fread(buf, file_size, 1, f);
    if(!items_read) {
        goto fail;
    }

    buf[file_size] = 0;
    fclose(f);

    return buf;

fail:
    if(f) fclose(f);
    if(buf) UT_FREE_MEMORY(buf);
    return NULL;
}

char *UT_read_entire_file_and_null_terminate_arena(char *file_path, UT_Arena *arena) {
    FILE *f = NULL;

    f = fopen(file_path, "r");
    if(!f) {
        perror(NULL);
        goto fail;
    }

    size_t file_size = UT_get_file_size(f);
    char *buf = (char *)UT_arena_alloc(arena, file_size + 1, sizeof *buf);
    size_t items_read = fread(buf, file_size, 1, f);
    if(!items_read) {
        // deallocate
        arena->cur -= file_size + 1;
        goto fail;
    }

    buf[file_size] = 0;
    fclose(f);
    return buf;

fail:
    if(f) fclose(f);
    return NULL;
}

UT_String UT_read_entire_file_as_string(UT_String file_path, UT_Arena *arena) {
    FILE *f = NULL;

    f = fopen(file_path.data, "r");
    if(!f) {
        perror(NULL);
        goto fail;
    }

    size_t file_size = UT_get_file_size(f);
    char *buf = (char *)UT_arena_alloc(arena, file_size + 1, 1);
    size_t items_read = fread(buf, file_size, 1, f);
    if(!items_read) {
        // deallocate
        arena->cur -= file_size + 1;
        goto fail;
    }

    buf[file_size] = 0;
    fclose(f);
    return UT_slice_to_string(buf, file_size);

fail:
    if(f) fclose(f);
    return (UT_String){0, 0};
}

UT_String *UT_list_directory(char *dir_path, size_t *out_length, UT_Arena *arena) {
    DIR *d;
    struct dirent *ep;
    d = opendir(dir_path);
    if(d) {
        int num_entries = 0;
        while((ep = readdir(d))) ++num_entries;
        UT_String *entry_names = UT_arena_alloc(arena, num_entries * sizeof(UT_String), sizeof(UT_String));
        *out_length = num_entries;
        rewinddir(d);
        for(int i = 0; (ep = readdir(d)); ++i) {
            entry_names[i] = UT_make_string(ep->d_name, strlen(ep->d_name), arena);
        }
        closedir(d);
        return entry_names;
    }
    *out_length = 0;
    return NULL;
}
#endif
