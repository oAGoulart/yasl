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

/**
  @namespace Memory
  @brief     Used for memory related functions
**/
namespace Memory
{

/**
  @brief  Force cast type

  This is one of the ways to cast a non-virtual member function pointer
  into a non-member pointer.

  @tparam To   Type to be cast into
  @tparam From Current type
  @param  in   Current value
  @retval      Cast type
  @warning Using this function may cause undefined behavior
**/
template<typename To, typename From>
inline To force_cast(From in)
{
  union {
    From in;  //!< Current type
    To out;   //!< Type to be cast into
  } u = { in };
  return u.out;
};

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
  class Detour {
  public:
    /**
      @brief  Check if pool is empty
      @retval Is pool empty?
    **/
    constexpr bool IsEmpty() const noexcept
    {
      return _pool.empty();
    }

    /**
      @brief  operator+=
      @param  f Function to be added to pool
      @retval   @c this object reference
    **/
    Detour& operator+=(const dummy_t& f)
    {
      _pool.push_back(f);
      return *this;
    }

    /**
      @brief  operator-=
      @param  f Function to be removed from pool
      @retval   @c this object reference
    **/
    Detour& operator-=(const dummy_t& f)
    {
      _pool.remove(f);
      return *this;
    }

    /**
      @brief  Operator used to iterate through pool
      @param  arg Forwarded args
      @retval     Return value
    **/
    T operator()(Args&&... arg)
    {
      T result;
      for (auto it = _pool.begin(); it != _pool.end(); ++it)
        result = (*it)(forward<Args>(arg)...);
      return result;
    }

  private:
    list<dummy_t> _pool;  //!< Pool of functions
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
  Trampoline(const uintptr_t& address, const size_t maxCalls = -1) :
    _TRAMPOLINE_HEAP_SIZE(48), _address(address), _maxCalls(maxCalls), _callCount(0),
    _p(address, _TRAMPOLINE_HEAP_SIZE), _enabled(true), _trampoline(nullptr)
  {
    if (!_maxCalls)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid arguments"));

    _trampoline = reinterpret_cast<dummy_t>(
      HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _TRAMPOLINE_HEAP_SIZE));
    if (_trampoline == nullptr)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tCan't allocate heap memory"));

    auto p_tram = reinterpret_cast<uintptr_t>(_trampoline);
    auto l_this = reinterpret_cast<uintptr_t>(this);
    auto p_func = force_cast<void*>(&Trampoline::Proxy);
    auto m_func = reinterpret_cast<uintptr_t>(p_func);
    Patch t(p_tram, _TRAMPOLINE_HEAP_SIZE);

#ifdef __X86_ARCH__
    t.mov(t.ecx, l_this);
    t.jmp(m_func);
#else
    t.pop(t.rcx);
    t.mov(t.rdx, l_this);
    t.push(t.rdx);
    t.push(t.rcx);
    t.movabs(t.rcx, m_func);
    t.jmp(t.rcx);
#endif

#ifdef __X86_ARCH__
    _p.jmp(p_tram);
#else
    _p.movabs(rcx, p_tram);
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
    if (_trampoline != nullptr)
      HeapFree(GetProcessHeap(), 0, _trampoline);
  }

  /**
    @brief Finish trampoline hook
  **/
  void Finish()
  {
    _p.Restore();
    _maxCalls = 0;
  }

  /**
    @brief Enabled trampoline hook
    @note  Only useful if trampoline was disabled
  **/
  void Enable()
  {
    if (!_enabled)
      Write(_address, _p.GetPayload(), _p.GetCount());
  }

  /**
    @brief Disable trampoline hook
  **/
  void Disable()
  {
    if (_enabled)
      Write(_address, _p.GetOriginal(), _p.GetCount());
  }

  /**
    @brief  Proxy function used to detour original function calls
    @param  arg Fowarded arguments
    @retval     Original return type
  **/
  T Proxy(Args&&... arg)
  {
    if (_maxCalls != -1 && _callCount >= _maxCalls) {
      Finish();
      return Call<T, Args...>(_address, forward<Args>(arg)...);
    }

    auto result = before(forward<Args>(arg)...);
    if (!replace.IsEmpty())
      result = replace(forward<Args>(arg)...);
    else {
      Disable();
      result = Call<T, Args...>(_address, forward<Args>(arg)...);
      Enable();
    }
    result = after(forward<Args>(arg)...);

    ++_callCount;
    return result;
  }

private:
  const size_t _TRAMPOLINE_HEAP_SIZE;  //!< Number of bytes to allocate on heap

  uintptr_t _address;    //!< Address of trampoline
  size_t    _maxCalls;   //!< Maximum amount of calls
  size_t    _callCount;  //!< Current call count
  Patch     _p;          //!< Pointer to patch object
  bool      _enabled;    //!< Is trampoline enabled?

  T (*_trampoline)(Args...);
};

/**
  @brief  Call and return
  @tparam T       Return type
  @tparam Args    Parameter pack type
  @param  address Function address
  @param  args    Parameter pack
  @retval         Function returned value
**/
template<typename T, typename... Args>
inline T Call(uintptr_t address, Args&&... args)
{
  return reinterpret_cast<T(*)(Args&&...)>(address)(forward<Args>(args)...);
}

}
