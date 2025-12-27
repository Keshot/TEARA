#ifndef _TEARA_CORE_HEAP_ARRAY_H_
#define _TEARA_CORE_HEAP_ARRAY_H_

#include "Core/Types.h"
#include <stdlib.h>

template <typename type>
struct HeapArray { 
    HeapArray()
        : Handle(0)
        , Len(0) 
    {
    }

    ~HeapArray()
    {
        if (Handle) {
            free(Handle);
        }
    }
    
    HeapArray(const HeapArray& Other) = delete;
    HeapArray(HeapArray&& Other)
    {
        if (&Other == this) {
            return;
        }

        Handle  = Other.Handle;
        Len     = Other.Len;

        Other.Handle    = 0;
        Other.Len       = 0;
    }
    
    HeapArray& operator=(const HeapArray&) = delete;
    HeapArray& operator=(HeapArray&& Other)
    {
        if (&Other == this) {
            return *this;
        }

        Handle  = Other.Handle;
        Len     = Other.Len;

        Other.Handle    = 0;
        Other.Len       = 0;

        return *this;
    }

    void* Alloc(i32 Size)
    {
        if (Handle) {
            Free();
        }

        Handle = (type*)malloc(Len);
        
        if (Handle) {
            Len = Size;
        }

        return Handle;
    }

    void Free()
    {
        if (Handle) {
            free(Handle);

            Len = 0;
        }
    }

    type*   Handle;
    i32     Len;
};

#endif