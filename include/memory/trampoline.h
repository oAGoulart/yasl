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
#include "assembly.h"
#include "process.h"
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

  Trampoline(const Pointer& ptr, const size_t maxCalls = -1) :
    before({}), replace({}), after({}), ptr_(ptr), maxCalls_(maxCalls), callCount_(0u),
    p_(ptr_), isEnabled_(true)
  {
    if (!maxCalls_)
      _throws("Invalid arguments");

    Data this_;
    this_.PushObject(Pointer::FromMethod<T>(&Trampoline::Proxy).ToValue());
    Data func_;
    func_.PushObject(Pointer::FromObject(this).ToValue());
    Patch t;

    t.Symbols({
      { "this", this_ },
      { "Proxy", func_ }
    });

#ifdef __X86__
    t.Assembly(R"("
      mov ecx, this
      jmp Proxy
    ")");
#else
    t.Assembly(R"("
      pop rcx
      mov rdx, this
      push rdx
      push rcx
      mov rcx, Proxy
      jmp rcx
    ")");
#endif

    Data heap_;
    heap_.PushObject(t.GetHeap().ToValue());
    p_.Symbols({
      { "Heap", heap_ }
    });

#ifdef __X86__
    p_.Assembly(R"("
      jmp Heap
    ")");
#else
    p_.Assembly(R"("
      mov rcx, Heap
      jmp rcx
    ")");
#endif
  }

  /**
    @brief Trampoline object destructor
  **/
  ~Trampoline()
  {
    p_.Disable();
  }

  /**
    @brief Enabled trampoline hook
    @note  Only useful if trampoline was disabled
  **/
  void Enable()
  {
    if (!isEnabled_)
      p_.Enable();
  }

  /**
    @brief Disable trampoline hook
  **/
  void Disable()
  {
    if (isEnabled_)
      p_.Disable();
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
      Disable();

    return result;
  }

private:
  Pointer  ptr_;
  size_t   maxCalls_;   //!< Maximum amount of calls
  size_t   callCount_;  //!< Current call count
  Patch    p_;
  bool     isEnabled_;    //!< Is trampoline enabled?
};

}
