#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef IUBPATCH_EXPORTS
        #ifdef __GNUC__
            #define IUBPATCH_API __attribute__((dllexport))
        #else
            #define IUBPATCH_API __declspec(dllexport)
        #endif
    #elif defined(IUBPATCH_SHARED)
        #ifdef __GNUC__
            #define IUBPATCH_API __attribute__((dllimport))
        #else
            #define IUBPATCH_API __declspec(dllimport)
        #endif
    #else
        #define IUBPATCH_API
    #endif
#else
    #if __GNUC__ >= 4 || defined(__clang__)
        #define IUBPATCH_API __attribute__((visibility("default")))
    #else
        #define IUBPATCH_API
    #endif
#endif

#include "iubpatch/version.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4251)
#endif

#ifdef _MSC_VER
    #pragma warning(pop)
#endif
