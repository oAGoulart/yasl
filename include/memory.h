/**
  @brief     Memory module
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

#include "base.h"

/**
  @namespace Memory
  @brief     Used for memory related functions
**/
namespace Memory
{

template<typename R, typename... Args>
inline R Call(uintptr_t address, Args&&... args)
{
  return reinterpret_cast<R(*)(Args&&...)>(address)(forward<Args>(args)...);
}

#ifdef __X86__
template<typename R, typename C, typename... Args>
inline R CallMethod(uintptr_t address, C this_, Args&&... args)
{
  return reinterpret_cast<R(__thiscall*)(C, Args&&...)>(address)(this_, forward<Args>(args)...);
}
#endif

};

// submodules
#include "memory/pointer.h"
#include "memory/protection.h"
#include "memory/process.h"
#include "memory/trampoline.h"
#include "memory/data.h"
