#pragma once
#include <iostream>
#include <cmath>
#include <vector>
#include <string>

// Advanced Junk Code Generator Macro
// Inserts random operations that do nothing but change the compiled byte-code.
// NOTE: __asm is NOT supported in x64 MSVC. Using volatile ops instead.
#ifdef _M_IX86
#define JUNK_CODE \
    __asm{ push eax } \
    __asm{ xor eax, eax } \
    __asm{ setz al } \
    __asm{ push eax } \
    __asm{ pop eax } \
    __asm{ pop eax } 
#else
#define JUNK_CODE \
    { \
        volatile int _jkasm = 0; \
        _jkasm ^= _jkasm; \
        _jkasm = (_jkasm == 0) ? 1 : 0; \
        (void)_jkasm; \
    }
#endif

#define JUNK_BLOCK_1 \
    { \
        int _jk1 = 432; \
        int _jk2 = 56; \
        for(int _i=0; _i<5; _i++) { _jk1 = (_jk1 * 3) + _jk2; } \
        if(_jk1 > 10000000) _jk1 = 0; \
    }

#define JUNK_BLOCK_2 \
    { \
        volatile float _fl1 = 3.14159f; \
        volatile float _fl2 = 2.71828f; \
        _fl1 = sin(_fl1) * cos(_fl2); \
        _fl2 = tan(_fl1) + _fl2; \
    }

#define RANDOM_OPS \
    JUNK_BLOCK_1 \
    JUNK_BLOCK_2 \
    JUNK_CODE
