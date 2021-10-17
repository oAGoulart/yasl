/*
  Copyright (c) 2021 Augusto Goulart
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
*/
#pragma once

#include "base.h"
#include "patch.h"
#include "data.h"

namespace Memory
{

template<typename T, typename... Args>
class Trampoline {
public:
  using dummy_t = T& (*)(Args&&...);

  class Detour {
  public:
    constexpr bool empty() const noexcept
    {
      return _pool.empty();
    }

    Detour& operator+=(const dummy_t& f)
    {
      _pool.emplace_back(f);
      return this;
    }

    Detour& operator-=(const dummy_t& f)
    {
      _pool.remove(f);
      return this;
    }

    T& operator()(T& (*iterator)(const dummy_t& f))
    {
      T result;
      for (auto it = _pool.begin(); it != _pool.end(); ++it)
        result = iterator(*it);
      return result;
    }

  private:
    list<dummy_t> _pool;
  };

  // assignable pools
  Detour before;
  Detour replace;
  Detour after;

  Trampoline(const uintptr_t& address, const size_t maxCalls = -1) :
    _address(address), _maxCalls(maxCalls), _callCount(0), _enabled(true)
  {
    if (!_maxCalls)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid arguments"));

    _trampoline = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _TRAMPOLINE_HEAP_SIZE);
    if (_trampoline == nullptr)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tCan't allocate heap memory"));

    auto l_this = reinterpret_cast<uint32_t>(this);
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

  ~Trampoline()
  {
    if (_enabled)
      Finish();
    HeapFree(GetProcessHeap(), 0, _trampoline);
  }

  void Finish()
  {
    _p->Restore();
    _maxCalls = 0;
  }

  void Enable()
  {
    if (!_enabled)
      Write(_address, _p->GetPayload(), _p->GetCount());
  }

  void Disable()
  {
    if (_enabled)
      Write(_address, _p->GetOriginal(), _p->GetCount());
  }

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
  uintptr_t         _address;
  size_t            _maxCalls;
  size_t            _callCount;
  unique_ptr<Patch> _p;
  bool              _enabled;

  void (*_trampoline)();

  const size_t _TRAMPOLINE_HEAP_SIZE 48;
};

template<typename T, typename... Args>
inline T& Call(uintptr_t address, Args&&... args)
{
  return reinterpret_cast<T & (*)(Args&&...)>(address)(forward<Args>(args)...);
}

inline void CallVoid(uintptr_t address)
{
  reinterpret_cast<void(*)()>(address)();
}

}
