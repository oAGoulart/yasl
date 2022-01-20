/**

  @brief     Protection submodule
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
#include "pointer.h"

namespace Memory
{

/**
  @class Protection
  @brief Object used to change memory virtual protection

  Changes page virtual protection on construction and reset to previous mode
  when this object gets destroyed.
**/
class Protection {
public:
  Protection(const Pointer& ptr, const size_t& size, const ulong_t& mode = PAGE_EXECUTE_READWRITE) :
    ptr_(ptr), size_(size), mode_(mode), oldMode_(0)
  {
    if (size_)
      isEnabled_ = VirtualProtect(&ptr_, size_, mode_, &oldMode_);
    isEnabled_ = false;
  };

  /**
    @brief Protection object destructor
  **/
  ~Protection()
  {
    if (isEnabled_)
      isEnabled_ = !VirtualProtect(&ptr_, size_, oldMode_, &oldMode_);
  };

  /**
    @brief  Check if protection change is enabled
    @retval bool Is change enabled?
  **/
  constexpr bool IsEnabled() const noexcept
  {
    return (isEnabled_);
  }

  /**
    @brief  Gets old mode
    @retval ulong_t Old mode
  **/
  constexpr ulong_t& GetOldMode() noexcept
  {
    return oldMode_;
  }

  /**
    @brief  Gets current mode
    @retval ulong_t& Current mode
  **/
  constexpr ulong_t& GetMode() noexcept
  {
    return mode_;
  }

private:
  Pointer ptr_;
  ulong_t mode_;      //!< Current mode
  ulong_t oldMode_;   //!< Old mode
  size_t  size_;      //!< Size of memory change
  lbool_t isEnabled_; //!< Is change enabled?
};

}
