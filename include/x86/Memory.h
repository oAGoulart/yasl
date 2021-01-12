#pragma once

#include "../Memory.h"

#ifdef __X86_ARCH__

namespace scriptwrapper
{

namespace memory
{

template<typename T, typename C, typename... Args>
class PatchMethod
{
public:
  data::List<T (__thiscall *)(C, Args...)> before;
  data::List<T (__thiscall *)(C, Args...)> replace;
  data::List<T (__thiscall *)(C, Args...)> after;

  PatchMethod(UINT_PTR address)
  {
    _target = address;
    _detour = [this](C _this, Args&&... args) {
      auto iterator = [&](T (__thiscall *func)(C, Args...)) {
        func(_this, std::forward<Args>(args));
      };

      before(iterator);
      if (replace.empty())
        _trampoline(_this, std::forward<Args>(args));
      else
        replace(iterator);
      after(iterator);
    };

    MH_CreateHookCast(_target, _detour, &_trampoline);
  }

  ~PatchMethod()
  {
    MH_RemoveHookCast(_target);
  }

private:
  UINT_PTR _target;
  T (__thiscall *_detour)(C, Args...);
  T (__thiscall *_trampoline)(C, Args...);
};

template<typename T, typename C, typename... Args>
inline T CallMethod(UINT_PTR address, C _this, Args&&... args)
{
  return reinterpret_cast<T (__thiscall *)(C, Args...)>(address)(_this, std::forward<Args>(args)...);
};

template<typename T, typename C, typename... Args>
inline T CallMethodDynamic(UINT_PTR address, C _this, Args&&... args)
{
  return reinterpret_cast<T (__thiscall *)(C, Args...)>(GetDynamicAddress(address))(_this, std::forward<Args>(args)...);
};

template<typename T, size_t index, typename C, typename... Args>
inline T CallMethodVirtual(C _this, Args&&... args)
{
  return reinterpret_cast<T (__thiscall *)(C, Args...)>(GetVmt(_this, index))(_this, std::forward<Args>(args)...);
};

}

}

#endif

