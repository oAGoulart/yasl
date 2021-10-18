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

/**
  @namespace Memory
  @brief     Used for memory related functions
**/
namespace Memory
{

/**
  @class Data
  @brief Object used to store data
**/
class Data : private vector<uint8_t> {
public:
  /**
    @brief Push byte into buffer
    @param value Value to be pushed
  **/
  constexpr void Push(const uint8_t& value)
  {
    push_back(value);
  }

  /**
    @brief  Get buffer length
    @retval Size of buffer
  **/
  constexpr size_t Size() const noexcept
  {
    return size();
  }

  /**
    @brief  Get buffer address
    @retval Buffer pointer
  **/
  constexpr const uint8_t* Buffer() const noexcept
  {
    return data();
  }

  /**
    @brief Destroy stored data
  **/
  constexpr void Clear() noexcept
  {
    clear();
  }

  /**
    @brief  Push object into buffer
    @tparam T     Object type
    @param  value Object value
    @retval       Object reference
  **/
  template<class T>
  constexpr T& PushObject(const T& value)
  {
    auto last = size();
    reserve(last + sizeof(T));
    return (*reinterpret_cast<T*>(&this[last]) = value);
  }

  /**
    @brief  Pop object from buffer
    @tparam T Object type
    @retval   Object reference
  **/
  template<class T>
  constexpr T& PopObject()
  {
    auto last = size();
    auto offset = last - sizeof(T);
    if (offset >= last)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tTried to pop object larger than vector size"));

    auto obj = *reinterpret_cast<T*>(&this[offset]);
    resize(offset);
    return obj;
  }

  /**
    @brief  Read object from buffer
    @tparam T      Object type
    @param  offset Offset in bytes from buffer base pointer
    @retval        Object reference
  **/
  template<class T>
  constexpr T& ReadObject(const size_t offset)
  {
    if (size() < offset + sizeof(T))
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tTried to read object larger than vector size"));
    return *reinterpret_cast<T*>(&this[offset]);
  }
};

/**
  @brief Read data from memory
  @param address Memory address
  @param data    Data object to store values
  @param count   Number of bytes to be read
  @param maxRead Maximum number of bytes to be read, this is
                 useful to prevent overflow
  @param vp      Is memory virtual protected?
**/
inline void Read(uintptr_t address, Data& data, const size_t count,
                 const size_t maxRead, const bool vp = true)
{
  if (maxRead <= data.Size() + count)
    throw runtime_error(_STRCAT(__FUNCSIG__, "\tTried to read beyond maximum allowed"));

  Protection protection(address, (vp) ? count : 0);
  for (size_t i = 0; i < count; ++i)
    data.Push(*reinterpret_cast<uint8_t*>(address + i));
};

/**
  @brief Write data into memory
  @param address Memory address
  @param data    Data object to read values from
  @param count   Number of bytes to write
  @param vp      Is memory virtual protected?
**/
inline void Write(uintptr_t address, Data& data, const size_t count, const bool vp = true)
{
  Protection protection(address, (vp) ? count : 0);
  memcpy_s(reinterpret_cast<uint8_t*>(address), count, data.Buffer(), data.Size());
};

/**
  @brief  Read object from memory
  @tparam T       Object type
  @param  address Memory address
  @param  vp      Is memory virtual protected?
  @retval         Object reference
**/
template<class T>
inline T& ReadObject(uintptr_t address, const bool vp = true)
{
  Protection protection(address, (vp) ? sizeof(T) : 0);
  return *reinterpret_cast<T*>(address);
};

/**
  @brief  Write object into memory
  @tparam T       Object type
  @param  address Memory address
  @param  value   Object reference
  @param  vp      Is memory virtual protected?
  @retval         Object reference
**/
template<class T>
inline T& WriteObject(uintptr_t address, const T& value, const bool vp = true)
{
  Protection protection(address, (vp) ? sizeof(T) : 0);
  return (*reinterpret_cast<T*>(address) = value);
};

/**
  @brief Fill memory with value
  @param address Memory address
  @param value   Value to be used
  @param size    Number of bytes to fill
  @param vp      Is memory virtual protected?
**/
inline void Fill(uintptr_t address, const uint8_t& value, const size_t size, const bool vp = true)
{
  Protection protection(address, (vp) ? size : 0);
  memset(reinterpret_cast<uint8_t*>(address), value, size);
};

}