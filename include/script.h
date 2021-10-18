/**
  @brief     Script module
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
  @class Script
  @brief Object used to manage scripts
**/
class Script {
public:
  /**
    @brief Script object constructor
    @param hmodule Handle to module
    @param func    Pointer to main function
    @param name    Script name string
  **/
  Script(const hmodule_t& hmodule, pfunc_t func, const path& name) :
    _module(hmodule), _name(name)
  {
    _main = reinterpret_cast<uintptr_t>(func);
  };

  /**
    @brief Script object destructor
  **/
  ~Script()
  {
    if (_module != nullptr)
      FreeLibrary(_module);
  };

  /**
    @brief  Gets script name
    @retval Script name
  **/
  path& GetName() noexcept
  {
    return _name;
  };

  /**
    @brief Call main function
  **/
  void operator()()
  {
    reinterpret_cast<void (*)()>(_main)();
  };

private:
  path      _name;   //!< Script name
  hmodule_t _module; //!< Handle to module
  uintptr_t _main;   //!< Main function address
};
