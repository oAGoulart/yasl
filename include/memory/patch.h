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
#include "data.h"
#include "peformat.h"

namespace Memory
{

enum class Register : uint8_t {
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
};

// careful: destroying this object won't restore original memory data
class Patch {
public:
  using enum Register;

  Patch(const uintptr_t& address, const size_t& maxRead) :
    _address(address), _offset(address), _maxRead(maxRead)
  {
    if (_address < _BASE_ADDRESS)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid address"));
  }

  constexpr size_t GetCount() const noexcept
  {
    return _offset - _address;
  }

  constexpr Data& GetOriginal() noexcept
  {
    return _original;
  }

  constexpr Data& GetPayload() noexcept
  {
    return _payload;
  }

  void Restore()
  {
    if (_original.Size()) {
      Write(_address, _original, _original.Size());
      _offset = _address;
    }
  };

  void mov(const Register r, const uint8_t value)
  {
    auto ub = _UB(r);
    switch (r) {
      case al:
      case cl:
      case dl:
      case bl:
      case ah:
      case ch:
      case dh:
      case bh:
        *this << '\xB0' + ub - _UB(al);
        break;

#ifndef __X86_ARCH__
      case spl:
      case bpl:
      case sil:
      case dil:
        *this << '\x40' << '\xB4' + ub - _UB(spl);
        break;

      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x41' << '\xB0' + ub - _UB(r8b);
        break;
#endif
      default:
        mov(r, static_cast<uint16_t>(value)); // check bigger registers
    }
    *this << value;
  }

  void mov(const Register r, const uint16_t value)
  {
    auto ub = _UB(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\xB8' + ub - _UB(ax);
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
        *this << '\x66' << '\x41' << '\xB8' + ub - _UB(r8w);
        break;
#endif
      default:
        mov(r, static_cast<uint32_t>(value)); // check bigger registers
    }
    *this << value;
  }

  void mov(const Register r, const uint32_t value)
  {
    auto ub = _UB(r);
    switch (r) {
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << '\xB8' + ub - _UB(eax);
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
        *this << '\x41' << '\xB8' + ub - _UB(r8d);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xC7' << '\xC0' + ub - _UB(rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xC7' << '\xC0' + ub - _UB(r8);
        break;
#endif
      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
    *this << value;
  }

  void mov(const uint32_t address, const Register r)
  {
    auto ub = _UB(r);
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
        *this << '\x88' << '\x0D' + (ub - _UB(cl)) * 8 << '\x12';
        break;

#ifndef __X86_ARCH__
      case spl:
      case bpl:
      case sil:
      case dil:
        *this << '\x40' << '\x88' << '\x24' + (ub - _UB(spl)) * 8 << '\x25';
        break;

      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x44' << '\x88' << '\x04' + (ub - _UB(r8b)) * 8 << '\x25';
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
        *this << '\x89' << '\x0D' + (ub - _UB(eax)) * 8;
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
        *this << '\x41' << '\xB8' + ub - _UB(r8d);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xC7' << '\xC0' + ub - _UB(rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xC7' << '\xC0' + ub - _UB(r8);
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
    auto ub = _UB(r);
    switch (r) {
      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xB8' + ub - _UB(rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xB8' + ub - _UB(r8);
        break;

      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
    *this << value;
  }
#endif

  void push(const Register r)
  {
    auto ub = _UB(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\x50' + ub - _UB(ax);
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
        *this << '\x50' + ub - _UB(eax);
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
        *this << '\x66' << '\x41' << '\x50' + ub - _UB(r8w);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x50' + ub - _UB(rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << '\x50' + ub - _UB(r8);
        break;
#endif
      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
  }

  void pop(const Register r)
  {
    auto ub = _UB(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\x58' + ub - _UB(ax);
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
        *this << '\x58' + ub - _UB(eax);
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
        *this << '\x66' << '\x41' << '\x58' + ub - _UB(r8w);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x58' + ub - _UB(rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << '\x58' + ub - _UB(r8);
        break;
#endif
      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
  }

  void jmp(void (*func)())
  {
    *this << '\xE9';
    auto addr = GetRelativeOffset(reinterpret_cast<uintptr_t>(func), _offset + 4);
    *this << addr;
  }

  void jmp(const Register r)
  {
    auto ub = _UB(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\xFF' << '\xE0' + ub - _UB(ax);
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
        *this << '\xFF' << '\xE0' + ub - _UB(eax);
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
        *this << '\x66' << '\x41' << '\xFF' << '\xE0' + ub - _UB(r8w);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\xFF' << '\xE0' + ub - _UB(rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << '\xFF' << '\xE0' + ub - _UB(r8);
        break;
#endif
      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
  }

  void call(void (*func)())
  {
    *this << '\xE8';
    auto addr = GetRelativeOffset(reinterpret_cast<uintptr_t>(func), _offset + 4);
    *this << addr;
  }

  void call(const Register r)
  {
    auto ub = _UB(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\xFF' << '\xD0' + ub - _UB(ax);
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
        *this << '\xFF' << '\xD0' + ub - _UB(eax);
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
        *this << '\x66' << '\x41' << '\xFF' << '\xD0' + ub - _UB(r8w);
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this '\xFF' << '\xD0' + ub - _UB(rax);
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << '\xFF' << '\xD0' + ub - _UB(r8);
        break;
#endif
      default:
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tInvalid register"));
    }
  }

  void nop(const size_t count)
  {
    Read(_offset, _original, count, _maxRead);
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

  void je(const uintptr_t address)
  {
    *this << '\x0F' << '\x84' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jz)(const uintptr_t) = je;

  void jne(const uintptr_t address)
  {
    *this << '\x0F' << '\x85' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jnz)(const uintptr_t) = jne;

  void jg(const uintptr_t address)
  {
    *this << '\x0F' << '\x8F' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jnle)(const uintptr_t) = jg;

  void jge(const uintptr_t address)
  {
    *this << '\x0F' << '\x8D' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jnl)(const uintptr_t) = jge;

  void jl(const uintptr_t address)
  {
    *this << '\x0F' << '\x8C' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jnge)(const uintptr_t) = jl;

  void jle(const uintptr_t address)
  {
    *this << '\x0F' << '\x8E' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jng)(const uintptr_t) = jle;

  // add
  // and
  // cmp
  // dec
  // div
  // inc
  // mul
  // neg
  // not
  // or
  // sub
  // test
  // xor

private:
  uintptr_t _address;
  uintptr_t _offset;
  Data      _original;
  Data      _payload;
  size_t    _maxRead;

  inline friend Patch& operator<<(Patch& p, const int8_t value)
  {
    return p << static_cast<uint8_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint8_t value)
  {
    Read(p._offset, p._original, 1, p._maxRead);
    p._payload.PushObject<uint8_t>(value);
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
    Read(p._offset, p._original, 2, p._maxRead);
    p._payload.PushObject<uint16_t>(value);
    WriteObject<uint16_t>(p._offset, value);
    p._offset += 2;
    return p;
  }

  inline friend Patch& operator<<(Patch& p, const int32_t value)
  {
    return p << static_cast<uint32_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint32_t value)
  {
    Read(p._offset, p._original, 4, p._maxRead);
    p._payload.PushObject<uint32_t>(value);
    WriteObject<uint32_t>(p._offset, value);
    p._offset += 4;
    return p;
  }

  inline friend Patch& operator<<(Patch& p, const int64_t value)
  {
    return p << static_cast<uint64_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint64_t value)
  {
    Read(p._offset, p._original, 8, p._maxRead);
    p._payload.PushObject<uint64_t>(value);
    WriteObject<uint64_t>(p._offset, value);
    p._offset += 8;
    return p;
  }
};


}
