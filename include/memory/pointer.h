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
  Pointer(const pvoid_t& void_)
  {
    pref_.pvoid = void_;
  }

  Pointer(const pfunc_t& func_)
  {
    pref_.pfunc = func_;
  }

  Pointer(const pbytes_t& bytes_)
  {
    pref_.pbytes = bytes_;
  }

  Pointer(const uintptr_t& value_)
  {
    pref_.value = value_;
  }

  Pointer(const nullptr_t& nullp_)
  {
    pref_.pvoid = nullp_;
  }

  template<typename T>
  static Pointer FromObject(T* obj_)
  {
    return Pointer::_ForceCast<pvoid_t>(obj_);
  }

  template<typename R, typename C>
  static Pointer FromMethod(R (C::*func_)())
  {
    return Pointer::_ForceCast<pvoid_t>(func_);
  }

  constexpr pvoid_t ToVoid() const noexcept
  {
    return pref_.pvoid;
  }

  constexpr pfunc_t ToFunc() const noexcept
  {
    return pref_.pfunc;
  }

  constexpr pbytes_t ToBytes() const noexcept
  {
    return pref_.pbytes;
  }

  constexpr uintptr_t ToValue() const noexcept
  {
    return pref_.value;
  }

  template<typename T>
  constexpr T* ToObject() const noexcept
  {
    return Pointer::_ForceCast<T*>(pref_.pvoid);
  }

  template<typename T>
  constexpr T ToAny() const noexcept
  {
    return Pointer::_ForceCast<T>(pref_.pvoid);
  }

  const Pointer& operator+=(const Pointer& ptr) noexcept
  {
    pref_.value += ptr.ToValue();
    return *this;
  }

  const Pointer& operator+=(const uintptr_t& address) noexcept
  {
    pref_.value += address;
    return *this;
  }

  const Pointer& operator++() noexcept
  {
    pref_.value++;
    return *this;
  }

  const Pointer& operator-=(const Pointer& ptr) noexcept
  {
    pref_.value -= ptr.ToValue();
    return *this;
  }

  const Pointer& operator-=(const uintptr_t& address) noexcept
  {
    pref_.value -= address;
    return *this;
  }

  const Pointer& operator--() noexcept
  {
    pref_.value--;
    return *this;
  }

  const Pointer operator+(const Pointer& ptr) const noexcept
  {
    return pref_.value + ptr.ToValue();
  }

  const uintptr_t operator+(const uintptr_t& address) const noexcept
  {
    return pref_.value + address;
  }

  const Pointer operator-(const Pointer& ptr) const noexcept
  {
    return pref_.value - ptr.ToValue();
  }

  const uintptr_t operator-(const uintptr_t& address) const noexcept
  {
    return pref_.value - address;
  }

  const bool operator==(const uintptr_t& address) const noexcept
  {
    return pref_.value == address;
  }

  const bool operator==(const Pointer& ptr) const noexcept
  {
    return pref_.value == ptr.ToValue();
  }

  const bool operator<(const uintptr_t& address) const noexcept
  {
    return pref_.value < address;
  }

  const bool operator<(const Pointer& ptr) const noexcept
  {
    return pref_.value < ptr.ToValue();
  }

  const bool operator>(const uintptr_t& address) const noexcept
  {
    return pref_.value > address;
  }

  const bool operator>(const Pointer& ptr) const noexcept
  {
    return pref_.value > ptr.ToValue();
  }

private:
  union _pref_t {
    pvoid_t   pvoid;
    pfunc_t   pfunc;
    pbytes_t  pbytes;
    uintptr_t value;
  } pref_;

  template<typename T, typename F>
  static T _ForceCast(F in_)
  {
    union { F in_; T out_; } u = { in_ };
    return u.out_;
  }
};

}
