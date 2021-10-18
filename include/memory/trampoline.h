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
  @class  Trampoline
  @brief  Object used to create a trampoline into procedure memory
  @tparam T    Return type
  @tparam Args Parameter pack type
**/
template<typename T, typename... Args>
class Trampoline {
public:
  using dummy_t = T& (*)(Args&&...);

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
    constexpr bool empty() const noexcept
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
      _pool.emplace_back(f);
      return this;
    }

    /**
      @brief  operator-=
      @param  f Function to be removed from pool
      @retval   @c this object reference
    **/
    Detour& operator-=(const dummy_t& f)
    {
      _pool.remove(f);
      return this;
    }

    /**
      @brief  Operator used to iterate through pool
      @param  iterator Iteration function
      @retval          Return value
    **/
    T& operator()(T& (*iterator)(const dummy_t& f))
    {
      T result;
      for (auto it = _pool.begin(); it != _pool.end(); ++it)
        result = iterator(*it);
      return result;
    }

  private:
    list<dummy_t> _pool;  //!< Pool storing functions
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
    _address(address), _maxCalls(maxCalls), _callCount(0), _enabled(true)
  {
    if (!_maxCalls)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid arguments"));

    _trampoline = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _TRAMPOLINE_HEAP_SIZE);
    if (_trampoline == nullptr)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tCan't allocate heap memory"));

    auto l_this = reinterpret_cast<uintptr_t>(this);
    auto m_func = reinterpret_cast<void(*)()>(this->*Proxy);
    Patch t(_trampoline, _TRAMPOLINE_HEAP_SIZE);

#ifdef __X86_ARCH__
    t.pop(ecx);
    t.mov(epi, l_this);
    t.push(epi);
    t.push(ecx);
    t.jmp(m_func);
#else
    t.pop(rcx);
    t.mov(rpi, l_this);
    t.push(rpi);
    t.push(rcx);
    t.movabs(rcx, m_func);
    t.jmp(rcx);
#endif

    _p = make_unique<Patch>(_address, _TRAMPOLINE_HEAP_SIZE);
#ifdef __X86_ARCH__
    _p->jmp(_trampoline);
#else
    _p->movabs(rcx, _trampoline);
    _p->jmp(rcx);
#endif
  }

  /**
    @brief Trampoline object destructor
  **/
  ~Trampoline()
  {
    if (_enabled)
      Finish();
    HeapFree(GetProcessHeap(), 0, _trampoline);
  }

  /**
    @brief Finish trampoline hook
  **/
  void Finish()
  {
    _p->Restore();
    _maxCalls = 0;
  }

  /**
    @brief Enabled trampoline hook
    @note  Only useful if trampoline was disabled
  **/
  void Enable()
  {
    if (!_enabled)
      Write(_address, _p->GetPayload(), _p->GetCount());
  }

  /**
    @brief Disable trampoline hook
  **/
  void Disable()
  {
    if (_enabled)
      Write(_address, _p->GetOriginal(), _p->GetCount());
  }

  /**
    @brief  Proxy function used to detour original function calls
    @param  arg Fowarded arguments
    @retval     Original return type
  **/
  T& Proxy(Args&&... arg)
  {
    if (_maxCalls != -1 && _callCount >= _maxCalls) {
      Finish();
      return Call<T, Args...>(_address, forward<Args>(arg)...);
    }

    auto it = [arg](dummy_func& f) -> T&
    {
      f(forward<Args>(arg)...);
    }

    T result;
    result = before(it);
    if (!replace.empty())
      result = replace(it);
    else {
      Disable();
      result = Call(_address, forward<Args>(arg)...);
      Enable();
    }
    result = after(it);

    ++_callCount;
    return result;
  }

private:
  uintptr_t         _address;    //!< Address of trampoline
  size_t            _maxCalls;   //!< Maximum amount of calls
  size_t            _callCount;  //!< Current call count
  unique_ptr<Patch> _p;          //!< Pointer to patch object
  bool              _enabled;    //!< Is trampoline enabled?

  void (*_trampoline)();

  const size_t _TRAMPOLINE_HEAP_SIZE 48;
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
inline T& Call(uintptr_t address, Args&&... args)
{
  return reinterpret_cast<T&(*)(Args&&...)>(address)(forward<Args>(args)...);
}

/**
  @brief  Call
  @tparam Args    Parameter pack type
  @param  address Function address
  @param  args    Parameter pack
**/
template<typename... Args>
inline void CallVoid(uintptr_t address, Args&&... args)
{
  reinterpret_cast<void(*)(Args&&...)>(address)(forward<Args>(args)...);
}

}
