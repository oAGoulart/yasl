#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <utility>

#if defined(__ILP32__) || defined(_ILP32) || defined(__i386__) || defined(_M_IX86) || defined(_X86_)
  #undef __X86_ARCH__
  #define __X86_ARCH__ 1
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
  #undef __MINGW__
  #define __MINGW__ 1

  #define __CPP_VERSION__ __cplusplus
#else
  #define __CPP_VERSION__ _MSVC_LANG
#endif

#if !defined(__cplusplus)
#error This project can only be compiled with C++
#elif !(__CPP_VERSION__ >= 201703L)
#error This project needs C++17 or above
#endif

#define DEPRECATED(m) __declspec(deprecated(m))

typedef ptrdiff_t PTR_DIFF;
