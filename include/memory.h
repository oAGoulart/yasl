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

#ifdef __X86_ARCH__
#define BASE_ADDRESS 0x400000
#else
#define BASE_ADDRESS 0x140000000
#endif

namespace Memory
{

class Protection {
public:
  Protection(uintptr_t address, const size_t& size,
             const ulong_t& mode = PAGE_EXECUTE_READWRITE)
  {
    _address = address;
    _size = size;
    if (_size)
      VirtualProtect(reinterpret_cast<pvoid_t>(_address), _size,
                     mode, &_oldProtect);
  };

  ~Protection()
  {
    if (_size)
      VirtualProtect(reinterpret_cast<pvoid_t>(_address), _size,
                     _oldProtect, &_oldProtect);
  };

private:
  uintptr_t _address;
  ulong_t   _oldProtect;
  size_t    _size;
};

inline int32_t GetRelativeOffset(uintptr_t dest, uintptr_t from)
{
  return static_cast<int32_t>(dest - from);
};

inline void Read(uintptr_t address, vector<uint8_t>* buff,
                 const size_t count, const size_t maxRead, const bool vp = true)
{
  // throw to prevent heap overflow
  if (maxRead <= buff->size() + count)
    throw runtime_error(_STRCAT(__FUNCSIG__, "\tTried to read beyond maximum allowed"));

  Protection protection(address, (vp) ? count : 0);
  for (size_t i = 0; i < count; ++i)
    buff->push_back(*reinterpret_cast<uint8_t*>(address + i));
};

inline void Write(uintptr_t address, vector<uint8_t>* buff,
                  const size_t count, const bool vp = true)
{
  Protection protection(address, (vp) ? count : 0);
  memcpy_s(reinterpret_cast<uint8_t*>(address), count, buff->data(), buff->size());
};

template<class T>
inline T& ReadObject(uintptr_t address, const bool vp = true)
{
  Protection protection(address, (vp) ? sizeof(T) : 0);
  return *reinterpret_cast<T*>(address);
};

template<class T>
inline T& WriteObject(uintptr_t address, const T& value, const bool vp = true)
{
  Protection protection(address, (vp) ? sizeof(value) : 0);
  return (*reinterpret_cast<T*>(address) = value);
};

inline void Fill(uintptr_t address, const uint8_t& value,
                 const size_t size, const bool vp = true)
{
  Protection protection(address, (vp) ? size : 0);
  memset(reinterpret_cast<uint8_t*>(address), value, size);
};

template<typename T, typename... Args>
inline T& Call(uintptr_t address, Args&&... args)
{
  return reinterpret_cast<T&(*)(Args&&...)>(address)(forward<Args>(args)...);
}

inline void CallVoid(uintptr_t address)
{
  reinterpret_cast<void(*)()>(address)();
}

// careful: destroying this object won't restore original memory data
class Patch {
public:
  typedef enum register_t : uint8_t {
    // byte registers
    al, cl, dl, bl, ah, ch, dh, bh,
#ifndef __X86_ARCH__
    spl, bpl, sil, dil,
    r8b, r9b, r10b, r11b, r12b, r13b, r14b, r15b,
#endif
    // word registers
    ax, cx, dx, bx, sp, bp, si, di,
#ifndef __X86_ARCH__
    r8w, r9w, r10w, r11w, r12w, r13w, r14w, r15w,
#endif
    // dword registers
    eax, ecx, edx, ebx, esp, ebp, esi, edi,
#ifndef __X86_ARCH__
    r8d, r9d, r10d, r11d, r12d, r13d, r14d, r15d,

    // qword registers
    rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi,
    r8, r9, r10, r11, r12, r13, r14, r15,
#endif
    noreg
  } Register;

  Patch(const uintptr_t& address, const size_t& maxRead)
  {
    if (address < BASE_ADDRESS)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid address"));

    _address = address;
    _offset = _address;
    _maxRead = maxRead;
  }

  size_t GetCount() const noexcept
  {
    return _offset - _address;
  }

  vector<uint8_t>& GetOriginal() noexcept
  {
    return _original;
  }

  void Restore()
  {
    if (_original.size()) {
      Write(_address, &_original, _original.size());
      _offset = _address;
    }
  };

  void mov(const Register r, const uint8_t value)
  {
    switch (r) {
      case al:
      case cl:
      case dl:
      case bl:
      case ah:
      case ch:
      case dh:
      case bh:
        *this << _UB('\xB0' + r - al);
        break;

#ifndef __X86_ARCH__
      case spl:
      case bpl:
      case sil:
      case dil:
        *this << '\x40' << _UB('\xB4' + r - spl);
        break;

      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x41' << _UB('\xB0' + r - r8b);
        break;
#endif
      default:
        mov(r, static_cast<uint16_t>(value)); // check bigger registers
    }
    *this << value;
  }

  void mov(const Register r, const uint16_t value)
  {
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << _UB('\xB8' + r - ax);
        break;

#ifndef __X86_ARCH__
      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41' << _UB('\xB8' + r - r8w);
        break;
#endif
      default:
        mov(r, static_cast<uint32_t>(value)); // check bigger registers
    }
    *this << value;
  }

  void mov(const Register r, const uint32_t value)
  {
    switch (r) {
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << _UB('\xB8' + r - eax);
        break;

#ifndef __X86_ARCH__
      case r8d:
      case r9d:
      case r10d:
      case r11d:
      case r12d:
      case r13d:
      case r14d:
      case r15d:
        *this << '\x41' << _UB('\xB8' + r - r8d);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xC7' << _UB('\xC0' + r - rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xC7' << _UB('\xC0' + r - r8);
        break;
#endif
      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
    *this << value;
  }

  void mov(const uint32_t address, const Register r)
  {
    switch (r) {
      case al:
        *this << '\xA2';
        break;

      case cl:
      case dl:
      case bl:
      case ah:
      case ch:
      case dh:
      case bh:
        *this << '\x88' << _UB('\x0D' + (r - cl) * 8) << '\x12';
        break;

#ifndef __X86_ARCH__
      case spl:
      case bpl:
      case sil:
      case dil:
        *this << '\x40' << '\x88' << _UB('\x24' + (r - spl) * 8) << '\x25';
        break;

      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x44' << '\x88' << _UB('\x04' + (r - r8b) * 8) << '\x25';
        break;
#endif
      case eax:
#ifdef __X86_ARCH__
        *this << '\xA3';
#else
        *this << '\x89' << '\x04';
#endif
        break;

      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << '\x89' << _UB('\x0D' + (r - eax) * 8);
        break;

#ifndef __X86_ARCH__
      case r8d:
      case r9d:
      case r10d:
      case r11d:
      case r12d:
      case r13d:
      case r14d:
      case r15d:
        *this << '\x41' << _UB('\xB8' + r - r8d);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xC7' << _UB('\xC0' + r - rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xC7' << _UB('\xC0' + r - r8);
        break;
#endif
      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
    *this << address;
  }

#ifndef __X86_ARCH__
  void movabs(const Register r, const uint64_t value)
  {
    switch (r) {
      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << _UB('\xB8' + r - rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << _UB('\xB8' + r - r8);
        break;

      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
    *this << value;
  }
#endif

  void push(const Register r)
  {
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << _UB('\x50' + r - ax);
        break;

#ifdef __X86_ARCH__
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << _UB('\x50' + r - eax);
        break;
#else
      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41' << _UB('\x50' + r - r8w);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << _UB('\x50' + r - rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << _UB('\x50' + r - r8);
        break;
#endif
      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
  }

  void pop(const Register r)
  {
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << _UB('\x58' + r - ax);
        break;

#ifdef __X86_ARCH__
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << _UB('\x58' + r - eax);
        break;
#else
      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41' << _UB('\x58' + r - r8w);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << _UB('\x58' + r - rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << _UB('\x58' + r - r8);
        break;
#endif
      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
  }

  void jmp(void (*func)())
  {
    *this << '\xE9';
    auto addr = GetRelativeOffset(_offset + 4, reinterpret_cast<uintptr_t>(func));
    *this << addr;
  }

  void jmp(const Register r)
  {
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\xFF' << _UB('\xE0' + r - ax);
        break;

#ifdef __X86_ARCH__
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << '\xFF' << _UB('\xE0' + r - eax);
        break;
#else
      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41' << '\xFF' << _UB('\xE0' + r - r8w);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\xFF' << _UB('\xE0' + r - rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << '\xFF' << _UB('\xE0' + r - r8);
        break;
#endif
      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
  }

  void call(void (*func)())
  {
    *this << '\xE8';
    auto addr = GetRelativeOffset(_offset + 4, reinterpret_cast<uintptr_t>(func));
    *this << addr;
  }

  void call(const Register r)
  {
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\xFF' << _UB('\xD0' + r - ax);
        break;

#ifdef __X86_ARCH__
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << '\xFF' << _UB('\xD0' + r - eax);
        break;
#else
      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41' << '\xFF' << _UB('\xD0' + r - r8w);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this '\xFF' << _UB('\xD0' + r - rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << '\xFF' << _UB('\xD0' + r - r8);
        break;
#endif
      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
  }

  void nop(const size_t count)
  {
    Read(_offset, &_original, count, _maxRead);
    Fill(_offset, '\x90', count);
    _offset += count;
  }

  void ret(const bool isFar = true)
  {
    *this << ((isFar) ? '\xCB' : '\xC3');
  }

  void int3()
  {
    *this << '\xCC';
  }

  void intr(const uint8_t code)
  {
    *this << '\xCD' << code;
  }

  // add
  // and
  // cmp
  // dec
  // div
  // inc
  // je
  // jg
  // jge
  // jl
  // jle
  // jne
  // mul
  // neg
  // not
  // or
  // sub
  // test
  // xor

private:
  uintptr_t       _address;
  uintptr_t       _offset;
  vector<uint8_t> _original;
  size_t          _maxRead;

  inline friend Patch& operator<<(Patch& p, const int8_t value)
  {
    return p << static_cast<uint8_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint8_t value)
  {
    Read(p._offset, &p._original, 1, p._maxRead);
    WriteObject<uint8_t>(p._offset, value);
    ++p._offset;
    return p;
  }

  inline friend Patch& operator<<(Patch& p, const int16_t value)
  {
    return p << static_cast<uint16_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint16_t value)
  {
    Read(p._offset, &p._original, 2, p._maxRead);
    WriteObject<uint16_t>(p._offset, value);
    ++p._offset;
    return p;
  }

  inline friend Patch& operator<<(Patch& p, const int32_t value)
  {
    return p << static_cast<uint32_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint32_t value)
  {
    Read(p._offset, &p._original, 4, p._maxRead);
    WriteObject<uint32_t>(p._offset, value);
    ++p._offset;
    return p;
  }

  inline friend Patch& operator<<(Patch& p, const int64_t value)
  {
    return p << static_cast<uint64_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint64_t value)
  {
    Read(p._offset, &p._original, 8, p._maxRead);
    WriteObject<uint64_t>(p._offset, value);
    ++p._offset;
    return p;
  }
};

template<typename T, typename... Args>
class Trampoline {
public:
  typedef T&(*dummy_func)(Args&&...);

  typedef class Detour {
  public:
    bool empty() const
    {
      return _pool.empty();
    }

    Detour& operator+=(const dummy_func& f)
    {
      _pool.emplace_back(f);
      return this;
    }

    Detour& operator-=(const dummy_func& f)
    {
      _pool.remove(f);
      return this;
    }

    T& operator()(T& (*iterator)(const dummy_func& f))
    {
      T result;
      for (auto it = _pool.begin(); it != _pool.end(); ++it)
        result = iterator(*it);
      return result;
    }

  private:
    list<dummy_func> _pool;
  };

  // assignable pools
  Detour before;
  Detour replace;
  Detour after;

  Trampoline(const uintptr_t& address, const size_t maxCalls = -1)
  {
    if (!maxCalls)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid arguments"));

    _address = address;
    _maxCalls = maxCalls;
    _callCount = 0;
    _enabled = true;

    _trampoline = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _TRAMPOLINE_HEAP_SIZE);
    if (_trampoline == nullptr)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tCan't allocate heap memory"));

    auto l_this = reinterpret_cast<uint32_t>(this);
    auto m_func = reinterpret_cast<void(*)()>(this->*Detour);
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
    Read(_address, &_hookCode, _p->GetCount());
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
      Write(_address, &_hookCode, _hookCode.size());
  }

  void Disable()
  {
    if (_enabled)
      Write(_address, _p->GetOriginal(), _p->GetCount());
  }

  T& Detour(Args&&... arg)
  {
    if (_maxCalls != -1 && _callCount >= _maxCalls) {
      Finish();
      return Call(_address, forward<Args>(arg)...);
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
  vector<uint8_t>   _hookCode;

  void (*_trampoline)();

  const size_t _TRAMPOLINE_HEAP_SIZE 48;
};

}
