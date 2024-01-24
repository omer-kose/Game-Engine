#pragma once

#include "defines.h"

// Disable assertions by commenting out the below line.
#define KASSERTIONS_ENABLED


#ifdef KASSERTIONS_ENABLED
#if _MSC_VER // Clang on Windows supports some of the Windows extensions and we need to take that into account if we are compiling with VS C/C++ compiler.
#include <intrin.h>
#define debugBreak() __debugbreak() // Halts the application whenever it is called (VS version)
#else
#define debugBreak() __builtin_trap() // (GCC version)
#endif


// As this is the only function of the header file we are not going to create a c file for it. Instead, we are going to define it in logger.h
KAPI void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line);


// # operator in the function (#expr) is called stringizing operator in C. It basically converts the arg into a string
#define KASSERT(expr)                                                \
    {                                                                \
        if (expr)                                                    \
        {                                                            \
        }                                                            \
        else                                                         \
        {                                                            \
            report_assertion_failure(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                            \
        }                                                            \
    }


#define KASSERT_MSG(expr, message)                                        \
    {                                                                     \
        if (expr)                                                         \
        {                                                                 \
        }                                                                 \
        else                                                              \
        {                                                                 \
            report_assertion_failure(#expr, message, __FILE__, __LINE__); \
            debugBreak();                                                 \
        }                                                                 \
    }


#ifdef _DEBUG
#define KASSERT_DEBUG(expr)                                          \
    {                                                                \
        if (expr)                                                    \
        {                                                            \
        }                                                            \
        else                                                         \
        {                                                            \
            report_assertion_failure(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                            \
        }                                                            \
    }
#else
#define KASSERT_DEBUG(expr)  // Does nothing at all
#endif

#else
#define KASSERT(expr)               // Does nothing at all
#define KASSERT_MSG(expr, message)  // Does nothing at all
#define KASSERT_DEBUG(expr)         // Does nothing at all
#endif
