/**
  @brief     Data submodule
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
#include "protection.h"
#include "pointer.h"

namespace Memory
{

class Data : private vector<ubyte_t> {
public:
  Data(initializer_list<ubyte_t> values) : vector<ubyte_t>(values)
  {
  }

  constexpr size_t Size() const noexcept
  {
    return size();
  }
  constexpr void Clear() noexcept
  {
    return clear();
  }

  const Pointer Bytes() noexcept
  {
    return data();
  }

  template<class T>
  const T& PushObject(const T& value)
  {
    auto last = size();
    resize(last + sizeof(value), 0u);
    return (*reinterpret_cast<T*>(data() + last) = value);
  }

  template<class T>
  const T PopObject()
  {
    auto last = size();
    auto offset = last - sizeof(T);
    if (offset >= last || sizeof(T) > last)
      _throws("Tried to pop object larger than vector size");

    auto obj = *reinterpret_cast<T*>(data() + offset - 1);
    resize(offset);
    return obj;
  }

  template<class T>
  const T& ReadObject(const size_t offset)
  {
    if (size() < offset + sizeof(T))
      _throws("Tried to read object larger than vector size");
    return *reinterpret_cast<T*>(data() + offset);
  }

  friend void Read(Pointer& ptr, Data& data, const size_t count, const bool vp = true)
  {
    Protection protection(ptr, (vp) ? count : 0);
    for (auto p = ptr; p < ptr + count; ++p)
      data.push_back(*p.ToBytes());
  }

  friend void Write(Pointer& ptr, Data& data, const size_t count, const bool vp = true)
  {
    Protection protection(ptr, (vp) ? count : 0);
    memcpy_s(ptr.ToVoid(), count, data.data(), data.size());
  }

  const ubyte_t& operator[](const size_t index) const
  {
    return at(index);
  }
};

template<class T>
inline T& ReadObject(Pointer& ptr, const bool vp = true)
{
  Protection protection(ptr, (vp) ? sizeof(T) : 0);
  return *ptr.ToObject<T>();
};

template<class T>
inline T& WriteObject(Pointer& ptr, const T& value, const bool vp = true)
{
  Protection protection(ptr, (vp) ? sizeof(T) : 0);
  return (*ptr.ToObject<T>() = value);
};

inline void Fill(Pointer& ptr, const ubyte_t& value, const size_t size, const bool vp = true)
{
  Protection protection(ptr, (vp) ? size : 0);
  memset(ptr.ToVoid(), value, size);
};

}
