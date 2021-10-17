/*
  Copyright (c) 2021 Augusto Goulart
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#pragma once

#include "pch.h"

#pragma comment(lib, "Imagehlp.lib")

#if defined(_M_IX86) || defined(_X86_) || defined(_WIN32)
#undef __X86_ARCH__
#define __X86_ARCH__ 1
#elif !defined(_M_AMD64) && !defined(_M_X64) && !defined(_WIN64)
#error "Unknown processor architecture"
#endif

#ifdef __cplusplus
  #if !(_MSVC_LANG >= 202002L)
  #error "Must compile with C++20 or higher"
  #endif
#else
#error "Must compile using C++"
#endif

#ifdef _DEBUG
#pragma warning("Do not use Debug version with a Release binary")
#endif

#ifndef _DLL
#error "Must compile this code targeting a shared library"
#endif

#ifdef __X86_ARCH__
#define _BASE_ADDRESS 0x400000
#else
#define _BASE_ADDRESS 0x140000000
#endif

#define _STRCATA(A, B) A ## B
#define _STRCAT(A, B) _STRCATA(A, B) // TODO: implement variadic func
#define _UB(A) static_cast<uint8_t>(A)

#define _STATIC_BUFF_SIZE MAX_PATH * 4
#define _CONFIG_SIZE_MAX INT32_MAX

using namespace std;
using namespace std::filesystem;

using long_t = LONG;
using ulong_t = DWORD;
using pvoid_t = LPVOID;
using pfunc_t = FARPROC;
using hmodule_t = HMODULE;
using lbool_t = BOOL;
using hfile_t = FILE;
using peimage_t = LOADED_IMAGE;
using exception_t = EXCEPTION_POINTERS;
using minidump_t = MINIDUMP_EXCEPTION_INFORMATION;
