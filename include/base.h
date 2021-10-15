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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <imagehlp.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <list>
#include <cwchar>

#pragma comment(lib, "Imagehlp.lib")

#if defined(__ILP32__) || defined(_ILP32) || defined(__i386__) || defined(_M_IX86) || defined(_X86_)
#undef __X86_ARCH__
#define __X86_ARCH__ 1
#endif

#define _STRCATA(A, B) A ## B
#define _STRCAT(A, B) _STRCATA(A, B)
#define _UB(A) static_cast<uint8_t>(A)

#define _STATIC_BUFF_SIZE MAX_PATH * 4

using namespace std;
using namespace std::filesystem;

typedef DWORD ulong_t;
typedef LPVOID pvoid_t;
typedef FARPROC pfunc_t;
typedef HMODULE hmodule_t;
typedef BOOL lbool_t;
typedef FILE hfile_t;

uint8_t operator""_u(char c)
{
  return static_cast<uint8_t>(c);
};

uint16_t operator""_u(wchar_t wc)
{
  return static_cast<uint16_t>(wc);
};
