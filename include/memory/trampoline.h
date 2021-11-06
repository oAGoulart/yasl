/**
  @brief     Trampoline submodule
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
#include "patch.h"
#include "data.h"
#include "pointer.h"
#include <initializer_list>

namespace Memory
{

/**
  @class  Trampoline
  @brief  Object used to create a trampoline into procedure memory
  @tparam T    Return type
  @tparam Args Parameter pack type
  @warning This object must not be destroyed before disabling the trampoline,
           doing it otherwise will cause undefined behavior
**/
template<typename T, typename... Args>
class Trampoline {
public:
  using dummy_t = T(*)(Args&&...);

  /**
    @class Detour
    @brief Object used for storing detour pool
  **/
  class Detour : private vector<dummy_t> {
  public:
    Detour(initializer_list<dummy_t> values) : vector<dummy_t>(values)
    {
    }

    constexpr bool Empty() const noexcept
    {
      return this->empty();
    }

    /**
      @brief  operator+=
      @param  f       Function to be added to pool
      @retval Detour& This object reference
    **/
    Detour& operator+=(const dummy_t& f)
    {
      this->push_back(f);
      return *this;
    }

    /**
      @brief  operator-=
      @param  f       Function to be removed from pool
      @retval Detour& This object reference
    **/
    Detour& operator-=(const dummy_t& f)
    {
      this->remove(f);
      return *this;
    }

    /**
      @brief  Operator used to iterate through pool
      @param  arg Forwarded args
      @retval T   Return value
    **/
    T operator()(Args&&... arg)
    {
      T result;
      for (auto it = this->begin(); it != this->end(); ++it)
        result = (*it)(forward<Args>(arg)...);
      return result;
    }
  };

  Detour before;  //!< Before original call
  Detour replace; //!< Replace original call
  Detour after;   //!< After original call

  /**
    @brief Trampoline object constructor
    @param address  Address of function to be hooked
    @param maxCalls Maximum amount of calls to catch, defaults to -1
                    which would be @c MAX_UINT - 1
  **/
  Trampoline(const PEFormat& image, const Pointer& ptr, const size_t maxCalls = -1) :
    before({}), replace({}), after({}),
    _heap_size(48u), _image(image), _ptr(ptr), _maxCalls(maxCalls), _callCount(0u),
    _p(_image, _ptr), _enabled(true), _trampoline(0u)
  {
    if (!_maxCalls)
      _throws("Invalid arguments");

    _trampoline = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _heap_size);
    if (_trampoline.ToVoid() == nullptr)
      _throws("Can't allocate heap memory");

    Pointer l_this = Pointer::FromObject(this);
    Pointer p_func = Pointer::FromAny(&Trampoline::Proxy);
    Patch t(_image, _trampoline);

#ifdef __X86_ARCH__
    t.mov(t.ecx, l_this.ToValue());
    t.jmp(p_func.ToValue());
#else
    t.pop(t.rcx);
    t.mov(t.rdx, l_this.ToValue());
    t.push(t.rdx);
    t.push(t.rcx);
    t.movabs(t.rcx, p_func.ToValue());
    t.jmp(t.rcx);
#endif

#ifdef __X86_ARCH__
    _p.jmp(p_func.ToValue());
#else
    _p.movabs(rcx, p_func.ToValue());
    _p.jmp(_p.rcx);
#endif
  }

  /**
    @brief Trampoline object destructor
  **/
  ~Trampoline()
  {
    if (_enabled)
      Finish();
    if (_trampoline.ToVoid() != nullptr)
      HeapFree(GetProcessHeap(), 0, _trampoline.ToVoid());
  }

  /**
    @brief Finish trampoline hook
  **/
  void Finish()
  {
    _p.Restore();
    _maxCalls = 0u;
  }

  /**
    @brief Enabled trampoline hook
    @note  Only useful if trampoline was disabled
  **/
  void Enable()
  {
    if (!_enabled)
      Write(_ptr, _p.GetPayload(), _p.GetCount());
  }

  /**
    @brief Disable trampoline hook
  **/
  void Disable()
  {
    if (_enabled)
      Write(_ptr, _p.GetOriginal(), _p.GetCount());
  }

  /**
    @brief  Proxy function used to detour original function calls
    @param  arg Fowarded arguments
    @retval T   Original return type
  **/
  T Proxy(Args&&... arg)
  {
    if (_maxCalls != -1 && _callCount >= _maxCalls) {
      Finish();
      return Call<T, Args...>(_ptr.ToValue(), forward<Args>(arg)...);
    }

    auto result = before(forward<Args>(arg)...);
    if (!replace.Empty())
      result = replace(forward<Args>(arg)...);
    else {
      Disable();
      result = Call<T, Args...>(_ptr.ToValue(), forward<Args>(arg)...);
      Enable();
    }
    result = after(forward<Args>(arg)...);

    ++_callCount;
    return result;
  }

private:
  const size_t _heap_size;  //!< Number of bytes to allocate on heap

  PEFormat _image;
  Pointer  _ptr;
  size_t   _maxCalls;   //!< Maximum amount of calls
  size_t   _callCount;  //!< Current call count
  Patch    _p;          //!< Pointer to patch object
  bool     _enabled;    //!< Is trampoline enabled?
  Pointer  _trampoline;
};

}
