#ifndef _TEARA_CORE_MEMORY_H_
#define _TEARA_CORE_MEMORY_H_

#include <stdlib.h>
#include <string.h>

template <typename type>
struct RawPointer {
    enum InitOptions { NoneInit, SetToZero };

    RawPointer()
        : Handle(0)
    {
    }

    RawPointer(i32 Size, InitOptions Opt)
        : Handle(0)
    {
        Alloc(Size, Opt);
    }

    ~RawPointer() {
        Free();
    }
    
    HeapArray(const HeapArray& Other) = delete;
    HeapArray(HeapArray&& Other) {
        Handle = Other.Handle;

        Other.Handle = 0;
    }
    
    HeapArray& operator=(const HeapArray&) = delete;
    HeapArray& operator=(HeapArray&& Other) {
        if (&Other == this) {
            return *this;
        }

        if (Handle) {
            free(Handle);
        }

        Handle = Other.Handle;

        Other.Handle = 0;

        return *this;
    }

    void* Alloc(i32 Size, InitOptions Opt) {
        Free();

        Handle = (type*)malloc(Size);

        switch (Opt) {
            case InitOptions::SetToZero : {
                memset(Handle, 0, Size);
            } break;

        }

        return Handle;
    }

    void Free() {
        if (Handle) {
            free(Handle);
        }
    }

    type* Release() {
        type* Tmp = Handle;

        Handle = 0;

        return Tmp;
    }

    type* Handle;
};

#endif