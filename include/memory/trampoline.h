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
    const Detour& operator+=(const dummy_t& f)
    {
      this->push_back(f);
      return *this;
    }

    /**
      @brief  operator-=
      @param  f       Function to be removed from pool
      @retval Detour& This object reference
    **/
    const Detour& operator-=(const dummy_t& f)
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

  Trampoline(const PEFormat& image, const Pointer& ptr, const size_t maxCalls = -1) :
    before({}), replace({}), after({}),
    _heapSize(48u), image_(image), ptr_(ptr), maxCalls_(maxCalls), callCount_(0u),
    p_(image_, ptr_), isEnabled_(true), trampoline_(0u)
  {
    if (!maxCalls_)
      _throws("Invalid arguments");

    trampoline_ = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _heapSize);
    if (trampoline_.ToVoid() == nullptr)
      _throws("Can't allocate heap memory");

    Pointer _this = Pointer::FromObject(this);
    Pointer _func = Pointer::FromMethod<T>(&Trampoline::Proxy);
    Patch t(image_, trampoline_);

#ifdef __X86__
    t.mov(t.ecx, _this.ToValue());
    t.jmp(_func.ToValue());
#else
    t.pop(t.rcx);
    t.mov(t.rdx, _this.ToValue());
    t.push(t.rdx);
    t.push(t.rcx);
    t.movabs(t.rcx, _func.ToValue());
    t.jmp(t.rcx);
#endif

#ifdef __X86__
    p_.jmp(trampoline_.ToValue());
#else
    p_.movabs(rcx, trampoline_.ToValue());
    p_.jmp(p_.rcx);
#endif
  }

  /**
    @brief Trampoline object destructor
  **/
  ~Trampoline()
  {
    if (isEnabled_)
      Finish();
    if (trampoline_.ToVoid() != nullptr)
      HeapFree(GetProcessHeap(), 0, trampoline_.ToVoid());
  }

  /**
    @brief Finish trampoline hook
  **/
  void Finish()
  {
    p_.Restore();
    maxCalls_ = 0u;
    isEnabled_ = false;
  }

  /**
    @brief Enabled trampoline hook
    @note  Only useful if trampoline was disabled
  **/
  void Enable()
  {
    if (!isEnabled_)
      Write(ptr_, p_.GetPayload(), p_.GetCount());
  }

  /**
    @brief Disable trampoline hook
  **/
  void Disable()
  {
    if (isEnabled_)
      Write(ptr_, p_.GetOriginal(), p_.GetCount());
  }

  /**
    @brief  Proxy function used to detour original function calls
    @param  arg Fowarded arguments
    @retval T   Original return type
  **/
  T Proxy(Args&&... arg)
  {
    auto result = before(forward<Args>(arg)...);
    if (!replace.Empty())
      result = replace(forward<Args>(arg)...);
    else {
      Disable();
      result = Call<T, Args...>(ptr_.ToValue(), forward<Args>(arg)...);
      Enable();
    }
    result = after(forward<Args>(arg)...);

    ++callCount_;
    if (maxCalls_ != -1 && callCount_ >= maxCalls_)
      Finish();

    return result;
  }

private:
  const size_t _heapSize;  //!< Number of bytes to allocate on heap

  PEFormat image_;
  Pointer  ptr_;
  size_t   maxCalls_;   //!< Maximum amount of calls
  size_t   callCount_;  //!< Current call count
  Patch    p_;
  bool     isEnabled_;    //!< Is trampoline enabled?
  Pointer  trampoline_;
};

}
