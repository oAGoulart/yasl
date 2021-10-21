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

/**
  @namespace Memory
  @brief     Used for memory related functions
**/
namespace Memory
{

/**
  @class Protection
  @brief Object used to change memory virtual protection
**/
class Protection {
public:
  /**
    @brief Protection object constructor
    @param address Memory address
    @param size    Length of memory section
    @param mode    Mode to change into
  **/
  Protection(const uintptr_t& address, const size_t& size,
             const ulong_t& mode = PAGE_EXECUTE_READWRITE) :
    _address(address), _size(size), _mode(mode)
  {
    if (_size)
      _isEnabled = VirtualProtect(reinterpret_cast<pvoid_t>(_address), _size, _mode, &_oldMode);
    _isEnabled = false;
  };

  /**
    @brief Protection object destructor
  **/
  ~Protection()
  {
    if (_isEnabled)
      _isEnabled = !VirtualProtect(reinterpret_cast<pvoid_t>(_address), _size, _oldMode, &_oldMode);
  };

  /**
    @brief  Check if protection change is enabled
    @retval Is change enabled?
  **/
  bool IsEnabled() const noexcept
  {
    return (_isEnabled);
  }

  /**
    @brief  Gets old mode
    @retval Old mode
  **/
  ulong_t GetOldMode() const noexcept
  {
    return _oldMode;
  }

  /**
    @brief  Gets current mode
    @retval Current mode
  **/
  ulong_t& GetMode() noexcept
  {
    return _mode;
  }

private:
  uintptr_t _address;   //!< Memory address
  ulong_t   _mode;      //!< Current mode
  ulong_t   _oldMode;   //!< Old mode
  size_t    _size;      //!< Size of memory change
  lbool_t   _isEnabled; //!< Is change enabled?
};

}
