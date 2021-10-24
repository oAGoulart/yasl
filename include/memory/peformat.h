/**
  @brief     PEFormat submodule
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

/**
  @brief  Calculate relative offset
  @param  dest Destination address
  @param  from Source address
  @retval      Relative address
**/
inline int32_t GetRelativeOffset(uintptr_t dest, uintptr_t from) noexcept
{
  return static_cast<int32_t>(dest - from);
};

/**
  @brief  Calculate absolute address
  @param  address Virtual address
  @retval         Absolute address
**/
inline uintptr_t GetAbsolute(uintptr_t address) noexcept
{
  return address + _BASE_ADDRESS;
};

/**
  @class PEFormat
  @brief Object used to store PE image values
**/
class PEFormat {
public:
  /**
    @brief PEFormat object constructor
    @param hmodule Module handle to be analyzed
  **/
  PEFormat(const hmodule_t& hmodule) : _module(hmodule)
  {
    _filename = make_unique<char[]>(MAX_PATH); // MapAndLoad doesn't support wchar_t
    if (!GetModuleFileNameA(hmodule, &_filename[0], MAX_PATH))
      _throws("Unable to find process filename");

    _image = make_unique<peimage_t>();
    if (!MapAndLoad(&_filename[0], NULL, &*_image, FALSE, TRUE))
      _throws("Could not map process binary file");
  }

  /**
    @brief PEFormat object destructor
  **/
  ~PEFormat()
  {
    UnMapAndLoad(&*_image);
  }

  /**
    @brief  Get entry point address
    @retval Entry address
  **/
  uintptr_t GetEntry() noexcept
  {
    return GetAbsolute(_image->FileHeader->OptionalHeader.AddressOfEntryPoint);
  }

private:
  hmodule_t             _module;   //!< Module handle
  unique_ptr<char[]>    _filename; //!< Module filename
  unique_ptr<peimage_t> _image;    //!< Module PE image
};

}
