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
#include "memory/pointer.h"

/**
  @class Script
  @brief Object used to manage scripts
**/
class Script {
public:
  /**
    @brief Script object constructor
    @param hmodule Handle to module where script is
    @param func    Pointer to script main function
    @param name    Script name string
  **/
  Script(const hmodule_t& hmodule, const Memory::Pointer& func, const path& name) :
    module_(hmodule), name_(name), main_(func)
  {
  }

  /**
    @brief Script object destructor
  **/
  ~Script()
  {
    if (module_ != nullptr)
      FreeLibrary(module_);
  }

  /**
    @brief  Gets script name
    @retval path& Script name
  **/
  constexpr path& GetName() noexcept
  {
    return name_;
  }

  /**
    @brief Call main function
  **/
  void operator()()
  {
    main_.ToFunc()();
  }

private:
  path            name_;   //!< Script name
  hmodule_t       module_; //!< Handle to module
  Memory::Pointer main_;
};
