/**
  @brief     Base module
  @author    Augusto Goulart
  @date      17.10.2021
  @copyright   Copyright (c) 2021 Augusto Goulart
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
**/
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <imagehlp.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <list>
#include <cwchar>
#include <sstream>

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

#if defined(_DEBUG) || !defined(NDEBUG)
#pragma message("Do not use Debug version with a Release binary")
#endif

#ifndef _DLL
#error "Must compile this code targeting a shared library"
#endif

using namespace std;
using namespace filesystem;

using long_t = LONG;
using ulong_t = DWORD;
using ubyte_t = uint8_t;
using pvoid_t = LPVOID;
using pfunc_t = FARPROC;
using pdata_t = ubyte_t*;
using handle_t = HANDLE;
using hmodule_t = HMODULE;
using lbool_t = BOOL;
using hfile_t = FILE;

#define \
_static_size MAX_PATH * 4

#define \
_crlf "\r\n"

#define \
_wcrlf L"\r\n"

#if defined(_SGR) || !defined(NSGR)
#define \
_esc "\x1B"

#define \
_csi \
  _esc "["

#define \
_osc \
  _esc "]"

/**
  @def   _format
  @brief Format string using escape sequence
  @param seq Sequence to be escaped
  @param str String to format
  @see https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#text-formatting
**/
#define \
_format(seq, str) \
  _csi seq  "m" str _csi "m"

#else
#define \
_format(seq, str) \
  str
#endif

/**
  @def   _throws
  @brief Throws runtime error with file, line, and message
  @param msg Error message to be shown
**/
#define \
_throws(msg) \
{ \
  string what(_format("33", "at file ")); \
  what += __FILE__; \
  what += _format("33", " on line ") + to_string(__LINE__) +  _crlf "\t"; \
  what += _format("37;41", "FAILURE\t") _format("31", msg) _crlf; \
  throw runtime_error(what); \
}

/**
  @def   _asserts
  @brief Asserts condition to be true, throws if not
  @param cond Condition to be evaluated
  @param msg  Message to show if thrown
**/
#define \
_asserts(cond, msg) \
{ \
  if (!(cond)) { \
    _throws(msg); \
  } \
}

#define \
_align(value) \
  __declspec(align(value))
