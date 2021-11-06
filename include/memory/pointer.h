/**
  @brief     Pointer submodule
  @author    Augusto Goulart
  @date      5.11.2021
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

namespace Memory
{

class Pointer {
public:
  Pointer(const pvoid_t& address)
  {
    _pref._pvoid = address;
  }

  Pointer(const pfunc_t& address)
  {
    _pref._pfunc = address;
  }

  Pointer(const pdata_t& address)
  {
    _pref._pdata = address;
  }

  Pointer(const uintptr_t& address)
  {
    _pref._value = address;
  }

  template<typename T>
  static Pointer FromObject(T* address)
  {
    return Pointer::_ForceCast<pvoid_t>(address);
  }

  // WARNING: Try not to use this method
  template<typename T>
  static Pointer FromAny(T address)
  {
    return Pointer::_ForceCast<pvoid_t>(&address);
  }

  pvoid_t ToVoid() const noexcept
  {
    return _pref._pvoid;
  }

  pfunc_t ToFunc() const noexcept
  {
    return _pref._pfunc;
  }

  pdata_t ToData() const noexcept
  {
    return _pref._pdata;
  }

  uintptr_t ToValue() const noexcept
  {
    return _pref._value;
  }

  template<typename T>
  T* ToObject() const noexcept
  {
    return Pointer::_ForceCast<T>(_pref._pvoid);
  }

  const Pointer& operator+=(const Pointer& ptr) noexcept
  {
    _pref._value += ptr.ToValue();
    return *this;
  }

  const Pointer& operator+=(const uintptr_t& address) noexcept
  {
    _pref._value += address;
    return *this;
  }

  const Pointer& operator++() noexcept
  {
    _pref._value++;
    return *this;
  }

  const Pointer& operator-=(const Pointer& ptr) noexcept
  {
    _pref._value -= ptr.ToValue();
    return *this;
  }

  const Pointer& operator-=(const uintptr_t& address) noexcept
  {
    _pref._value -= address;
    return *this;
  }

  const Pointer& operator--() noexcept
  {
    _pref._value--;
    return *this;
  }

  const Pointer operator+(const Pointer& ptr) const noexcept
  {
    return _pref._value + ptr.ToValue();
  }

  const uintptr_t operator+(const uintptr_t& address) const noexcept
  {
    return _pref._value + address;
  }

  const Pointer operator-(const Pointer& ptr) const noexcept
  {
    return _pref._value - ptr.ToValue();
  }

  const uintptr_t operator-(const uintptr_t& address) const noexcept
  {
    return _pref._value - address;
  }

  const bool operator==(const uintptr_t& address) const noexcept
  {
    return _pref._value == address;
  }

  const bool operator==(const Pointer& ptr) const noexcept
  {
    return _pref._value == ptr.ToValue();
  }

  const bool operator<(const uintptr_t& address) const noexcept
  {
    return _pref._value < address;
  }

  const bool operator<(const Pointer& ptr) const noexcept
  {
    return _pref._value < ptr.ToValue();
  }

  const bool operator>(const uintptr_t& address) const noexcept
  {
    return _pref._value > address;
  }

  const bool operator>(const Pointer& ptr) const noexcept
  {
    return _pref._value > ptr.ToValue();
  }

private:
  union _pref_t {
    pvoid_t   _pvoid;
    pfunc_t   _pfunc;
    pdata_t   _pdata;
    uintptr_t _value;
  } _pref;

  template<typename T, typename F>
  static T* _ForceCast(F* in)
  {
    union { F* in; T* out; } u = { in };
    return u.out;
  }
};

}
