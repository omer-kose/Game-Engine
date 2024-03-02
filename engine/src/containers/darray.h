#pragma once

#include "defines.h"

/*
    Memory layout
    u64 capacity = number elements that can be held
    u64 length = number of elements currently contained
    u64 stride = size of each element in bytes
    void* elements
*/

// Darray Fields (u64)
/*
    These fields are used to keep track of the internal state of the darray instance, like what is the current number of variables we have or 
    what is the stride so that we can fetch the values. 

    These are prepended before the actual data buffer as a header and used internally. However, caller will never see that
    header and will only see and use the actual data buffer part as intended.

    This is very similar to using private variables in classes. User never sees or touches them but internally they are used.
*/
enum 
{
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_LENGTH // Number of fields we have, standard enum size identifier not explicitly used.
};

/*
    Notice the _ naming convention that is used at the beginning of the variable names. This convention is used to denote that
    these functions are not explicitly called by the users.

    Note: We will still export them into the DLL as they will be indirectly called via Macros we defined
*/
KAPI void* _darray_create(u64 length, u64 stride);
KAPI void _darray_destroy(void* array);

KAPI u64 _darray_field_get(void* array, u64 field);
KAPI void _darray_field_set(void* array, u64 field, u64 value);

KAPI void* _darray_resize(void* array);

KAPI void* _darray_push(void* array, const void* value_ptr);
KAPI void _darray_pop(void* array, void* dest);

KAPI void* _darray_insert_at(void* array, u64 index, void* value_ptr);
KAPI void* _darray_pop_at(void* array, u64 index, void* dest);

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR 2

#define darray_create(type) \
    _darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type))

#define darray_reserve(type, capacity) \
    _darray_create(capacity, sizeof(type))

#define darray_destroy(array) _darray_destroy(array)

#define darray_push(array, value)           \
    {                                       \
        typeof(value) temp = value;         \
        array = _darray_push(array, &temp); \
    }

// NOTE: could use __auto_type for temp above, but intellisense
// for VSCode flags it as an unknown type. typeof() seems to
// work just fine, though. Both are GNU extensions.

#define darray_pop(array, value_ptr) \
    _darray_pop(array, value_ptr)

#define darray_insert_at(array, index, value)           \
    {                                                   \
        typeof(value) temp = value;                     \
        array = _darray_insert_at(array, index, &temp); \
    }

#define darray_pop_at(array, index, value_ptr) \
    _darray_pop_at(array, index, value_ptr)

#define darray_clear(array) \
    _darray_field_set(array, DARRAY_LENGTH, 0)

#define darray_capacity(array) \
    _darray_field_get(array, DARRAY_CAPACITY)

#define darray_length(array) \
    _darray_field_get(array, DARRAY_LENGTH)

#define darray_stride(array) \
    _darray_field_get(array, DARRAY_STRIDE)

#define darray_length_set(array, value) \
    _darray_field_set(array, DARRAY_LENGTH, value)