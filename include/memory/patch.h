/**
  @brief     Patch submodule
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
#include "data.h"

namespace Memory
{

/**
  @brief  Calculate relative offset
  @param  dest    Destination address
  @param  from    Source address
  @retval int32_t Relative address
**/
inline int32_t GetRelativeOffset(uintptr_t dest, uintptr_t from) noexcept
{
  return static_cast<int32_t>(dest - from);
};

/**
  @brief  Calculate absolute address
  @param  address   Virtual address
  @retval uintptr_t Absolute address
**/
inline uintptr_t GetAbsolute(uintptr_t address) noexcept
{
  return address + _BASE_ADDRESS;
};

/**
  @enum  Memory::Register
  @brief List of Assembly x86-32 and x86-64 registers
  @note  Members value is used to calcule some opcodes binary value, changing
         this @c enum order will cause undefined behavior
**/
enum class Register : uint8_t {
  //!<  Byte sized registers
  al, cl, dl, bl, ah, ch, dh, bh,
#ifndef __X86_ARCH__
  spl, bpl, sil, dil,
  r8b, r9b, r10b, r11b, r12b, r13b, r14b, r15b,
#endif
  //!<  Word sized registers
  ax, cx, dx, bx, sp, bp, si, di,
#ifndef __X86_ARCH__
  r8w, r9w, r10w, r11w, r12w, r13w, r14w, r15w,
#endif
  //!<  Dword sized registers
  eax, ecx, edx, ebx, esp, ebp, esi, edi,
#ifndef __X86_ARCH__
  r8d, r9d, r10d, r11d, r12d, r13d, r14d, r15d,
  //!<  Qword sized registers
  rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi,
  r8, r9, r10, r11, r12, r13, r14, r15,
#endif
  noreg //!<  End of registers
};

/**
  @class   Patch
  @brief   Object used to patch memory data
  @details This @c class has many function members that allow for
           patching current procedure memory
  @warning Destroying this object won't restore original memory data
**/
class Patch {
public:
  using enum Register;

  /**
    @brief Patch object constructor
    @param address Address on memory to be patched
    @param maxRead Maximum amount of bytes to be patched, this can
                   be used for preventing overflow depending on how
                   much space you have to apply the patch
  **/
  Patch(const uintptr_t& address, const size_t& maxRead) :
    _address(address), _offset(address), _maxRead(maxRead)
  {
    if (_address < _BASE_ADDRESS)
      _throws("Invalid address");
  }

  /**
    @brief  Gets size of patch
    @retval size_t Number of bytes written into memory
  **/
  constexpr size_t GetCount() const noexcept
  {
    return _offset - _address;
  }

  /**
    @brief  Get original data
    @retval Data& Original Data object, which contains the buffer storing
            the data replaced by the patch
  **/
  constexpr Data& GetOriginal() noexcept
  {
    return _original;
  }

  /**
    @brief  Get payload data
    @retval Data& Current payload Data object with patch values
  **/
  constexpr Data& GetPayload() noexcept
  {
    return _payload;
  }

  /**
    @brief Restore original data into memory
    @note  Calling this function will clear the payload buffer
           and subsequent patches will start from @c _address
  **/
  void Restore()
  {
    if (_original.Size()) {
      Write(_address, _original, _original.Size());
      _offset = _address;
      _payload.Clear();
    }
  };

  /**
    @brief Move byte into register

    Moves @p value into register @p r, if register is bigger
    than @c uint8_t then value will be cast into a bigger type

    @param r     Register destination
    @param value Byte value to be moved
  **/
  void mov(const Register r, const uint8_t value)
  {
    auto ub = _ubyte(r);
    switch (r) {
      case al:
      case cl:
      case dl:
      case bl:
      case ah:
      case ch:
      case dh:
      case bh:
        *this << _ubyte('\xB0' + ub - _ubyte(al));
        break;

#ifndef __X86_ARCH__
      case spl:
      case bpl:
      case sil:
      case dil:
        *this << '\x40' << _ubyte('\xB4' + ub - _ubyte(spl));
        break;

      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x41' << _ubyte('\xB0' + ub - _ubyte(r8b));
        break;
#endif
      default:
        mov(r, static_cast<uint16_t>(value));
    }
    *this << value;
  }

  /**
    @brief Move word into register

    Moves @p value into register @p r, if register is bigger
    than @c uint16_t then value will be cast into a bigger type

    @param r     Register destination
    @param value Word value to be moved
  **/
  void mov(const Register r, const uint16_t value)
  {
    auto ub = _ubyte(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << _ubyte('\xB8' + ub - _ubyte(ax));
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
        *this << '\x66' << '\x41' << _ubyte('\xB8' + ub - _ubyte(r8w));
        break;
#endif
      default:
        mov(r, static_cast<uint32_t>(value));
    }
    *this << value;
  }

  /**
    @brief Move dword into register

    Moves @p value into register @p r which should be either
    a dword or qword register, if you need to move a qword value
    then @see Memory::Patch::movabs

    @param r     Register destination
    @param value Dword value to be moved
  **/
  void mov(const Register r, const uint32_t value)
  {
    auto ub = _ubyte(r);
    switch (r) {
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << _ubyte('\xB8' + ub - _ubyte(eax));
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
        *this << '\x41' << _ubyte('\xB8' + ub - _ubyte(r8d));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xC7' << _ubyte('\xC0' + ub - _ubyte(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xC7' << _ubyte('\xC0' + ub - _ubyte(r8));
        break;
#endif
      default:
        _throws("Invalid register");
    }
    *this << value;
  }

  /**
    @brief Move register value into address
    @param address Destination of value
    @param r       Register storing value
  **/
  void mov(const uint32_t address, const Register r)
  {
    auto ub = _ubyte(r);
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
        *this << '\x88' << '\x0D' + _ubyte((ub - _ubyte(cl)) * 8) << '\x12';
        break;

#ifndef __X86_ARCH__
      case spl:
      case bpl:
      case sil:
      case dil:
        *this << '\x40' << '\x88' << '\x24' + _ubyte((ub - _ubyte(spl)) * 8) << '\x25';
        break;

      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x44' << '\x88' << '\x04' + _ubyte((ub - _ubyte(r8b)) * 8) << '\x25';
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
        *this << '\x89' << _ubyte('\x0D' + (ub - _ubyte(eax)) * 8);
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
        *this << '\x41' << _ubyte('\xB8' + ub - _ubyte(r8d));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xC7' << _ubyte('\xC0' + ub - _ubyte(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xC7' << _ubyte('\xC0' + ub - _ubyte(r8));
        break;
#endif
      default:
        _throws("Invalid register");
    }
    *this << address;
  }

#ifndef __X86_ARCH__
  /**
    @brief Move value into register

    Move unsigned 64bit value into qword register

    @param r     Register destination
    @param value Value to be moved
  **/
  void movabs(const Register r, const uint64_t value)
  {
    auto ub = _ubyte(r);
    switch (r) {
      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << _ubyte('\xB8' + ub - _ubyte(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << _ubyte('\xB8' + ub - _ubyte(r8));
        break;

      default:
        _throws("Invalid register");
    }
    *this << value;
  }
#endif

  /**
    @brief Push register into stack
    @param r Register to be pushed
  **/
  void push(const Register r)
  {
    auto ub = _ubyte(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << _ubyte('\x50' + ub - _ubyte(ax));
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
        *this << _ubyte('\x50' + ub - _ubyte(eax));
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
        *this << '\x66' << '\x41' << _ubyte('\x50' + ub - _ubyte(r8w));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << _ubyte('\x50' + ub - _ubyte(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << _ubyte('\x50' + ub - _ubyte(r8));
        break;
#endif
      default:
        _throws("Invalid register");
    }
  }

  /**
    @brief Pop register from stack
    @param r Register to be populated
  **/
  void pop(const Register r)
  {
    auto ub = _ubyte(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << _ubyte('\x58' + ub - _ubyte(ax));
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
        *this << _ubyte('\x58' + ub - _ubyte(eax));
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
        *this << '\x66' << '\x41' << _ubyte('\x58' + ub - _ubyte(r8w));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << _ubyte('\x58' + ub - _ubyte(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << _ubyte('\x58' + ub - _ubyte(r8));
        break;
#endif
      default:
        _throws("Invalid register");
    }
  }

  /**
    @brief Relative jump into function
    @param address Destination of jump
    @note  This function uses a 32bit relative address for the jump,
           if you need to make an absolute jump use a register instead
  **/
  void jmp(const uintptr_t address)
  {
    *this << '\xE9';
    auto addr = GetRelativeOffset(address, _offset + 4);
    *this << addr;
  }

  /**
    @brief Absolute jump into address
    @param r Register containing destination address
  **/
  void jmp(const Register r)
  {
    auto ub = _ubyte(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\xFF' << _ubyte('\xE0' + ub - _ubyte(ax));
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
        *this << '\xFF' << _ubyte('\xE0' + ub - _ubyte(eax));
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
        *this << '\x66' << '\x41' << '\xFF' << _ubyte('\xE0' + ub - _ubyte(r8w));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\xFF' << _ubyte('\xE0' + ub - _ubyte(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << '\xFF' << _ubyte('\xE0' + ub - _ubyte(r8));
        break;
#endif
      default:
        _throws("Invalid register");
    }
  }

  /**
    @brief Relative call function
    @param address Function to be called
    @note  This function casts the address and makes it
           relative to the patch location, if you need an absolute
           call use a register instead
  **/
  void call(const uintptr_t address)
  {
    *this << '\xE8';
    auto addr = GetRelativeOffset(address, _offset + 4);
    *this << addr;
  }

  /**
    @brief Absolute call function
    @param r Register containing function address
  **/
  void call(const Register r)
  {
    auto ub = _ubyte(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\xFF' << _ubyte('\xD0' + ub - _ubyte(ax));
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
        *this << '\xFF' << _ubyte('\xD0' + ub - _ubyte(eax));
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
        *this << '\x66' << '\x41' << '\xFF' << _ubyte('\xD0' + ub - _ubyte(r8w));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this '\xFF' << _ubyte('\xD0' + ub - _ubyte(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << '\xFF' << _ubyte('\xD0' + ub - _ubyte(r8));
        break;
#endif
      default:
        _throws("Invalid register");
    }
  }

  /**
    @brief Nop patch
    @param count Number of bytes to nop
  **/
  void nop(const size_t count)
  {
    Read(_offset, _original, count, _maxRead);
    Fill(_offset, '\x90', count);
    _offset += count;
  }

  /**
    @brief Return from call/jump
    @param isFar Is return far?
  **/
  void ret(const bool isFar = true)
  {
    *this << ((isFar) ? '\xCB' : '\xC3');
  }

  /**
    @brief Interrupt procedure (special debug opcode)
  **/
  void int3()
  {
    *this << '\xCC';
  }

  /**
    @brief Interrupt procedure
    @param code Code of interruption
    @see   https://pdos.csail.mit.edu/6.828/2005/lec/lec8-slides.pdf
  **/
  void intr(const uint8_t code)
  {
    *this << '\xCD' << code;
  }

  /**
    @brief Jump if equal
    @param address Destination address
  **/
  void je(const uintptr_t address)
  {
    *this << '\x0F' << '\x84' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jz)(const uintptr_t) = &Patch::je;  //!< Jump if zero

  /**
    @brief Jump if not equal
    @param address Destination address
  **/
  void jne(const uintptr_t address)
  {
    *this << '\x0F' << '\x85' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jnz)(const uintptr_t) = &Patch::jne;  //!< Jump if not zero

  /**
    @brief Jump if greater
    @param address Destination address
  **/
  void jg(const uintptr_t address)
  {
    *this << '\x0F' << '\x8F' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jnle)(const uintptr_t) = &Patch::jg;  //!< Jump if not less or equal

  /**
    @brief Jump if greater or equal
    @param address Destination address
  **/
  void jge(const uintptr_t address)
  {
    *this << '\x0F' << '\x8D' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jnl)(const uintptr_t) = &Patch::jge;  //!< Jump if not less

  /**
    @brief Jump if less
    @param address Destination address
  **/
  void jl(const uintptr_t address)
  {
    *this << '\x0F' << '\x8C' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jnge)(const uintptr_t) = &Patch::jl;  //!< Jump if not greater or equal

  /**
    @brief Jump less or equal
    @param address Destination address
  **/
  void jle(const uintptr_t address)
  {
    *this << '\x0F' << '\x8E' << GetRelativeOffset(address, _offset + 4);
  }
  void (Patch::*jng)(const uintptr_t) = &Patch::jle;  //!< Jump not greater

  /**
    @brief Increase register value by one
    @param r Register to be increased
  **/
  void inc(const Register r)
  {
    auto ub = _ubyte(r);
    switch (r) {
      case al:
      case cl:
      case dl:
      case bl:
      case ah:
      case ch:
      case dh:
      case bh:
        *this << '\xFE' << _ubyte('\xC0' + ub - _ubyte(al));
        break;

      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
#ifdef __X86_ARCH__
        *this << '\x66' << _ubyte('\x40' + ub - _ubyte(ax));
#else
        *this << '\x66' << '\xFF' << _ubyte('\xC0' + ub - _ubyte(ax));
#endif
        break;

      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
#ifdef __X86_ARCH__
        *this << _ubyte('\x40' + ub - _ubyte(eax));
#else
        *this << '\xFF' << _ubyte('\xC0' + ub - _ubyte(ax));
#endif
        break;

#ifndef __X86_ARCH__
      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x41' << '\xFE' << _ubyte('\xC0' + ub - _ubyte(r8b));
        break;

      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41' << '\xFF' << _ubyte('\xC0' + ub - _ubyte(r8w));
        break;

      case r8d:
      case r9d:
      case r10d:
      case r11d:
      case r12d:
      case r13d:
      case r14d:
      case r15d:
        *this << '\x41' << '\xFF' << _ubyte('\xC0' + ub - _ubyte(r8d));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xFF' << _ubyte('\xC0' + ub - _ubyte(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xFF' << _ubyte('\xC0' + ub - _ubyte(r8));
        break;
#endif
      default:
        _throws("Invalid register");
    }
  }

  /**
    @brief Decrease register value by one
    @param r Register to decrease
  **/
  void dec(const Register r)
  {
    auto ub = _ubyte(r);
    switch (r) {
      case al:
      case cl:
      case dl:
      case bl:
      case ah:
      case ch:
      case dh:
      case bh:
        *this << '\xFE' << _ubyte('\xC8' + ub - _ubyte(al));
        break;

      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
#ifdef __X86_ARCH__
        *this << '\x66' << _ubyte('\x48' + ub - _ubyte(ax));
#else
        *this << '\x66' << '\xFF' << _ubyte('\xC8' + ub - _ubyte(ax));
#endif
        break;

      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
#ifdef __X86_ARCH__
        *this << _ubyte('\x48' + ub - _ubyte(eax));
#else
        *this << '\xFF' << _ubyte('\xC8' + ub - _ubyte(eax));
#endif
        break;

#ifndef __X86_ARCH__
      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x41' << '\xFE' << _ubyte('\xC8' + ub - _ubyte(r8b));
        break;

      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41' << '\xFF' << _ubyte('\xC8' + ub - _ubyte(r8w));
        break;

      case r8d:
      case r9d:
      case r10d:
      case r11d:
      case r12d:
      case r13d:
      case r14d:
      case r15d:
        *this << '\x41' << '\xFF' << _ubyte('\xC8' + ub - _ubyte(r8d));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xFF' << _ubyte('\xC8' + ub - _ubyte(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xFF' << _ubyte('\xC8' + ub - _ubyte(r8));
        break;
#endif
      default:
        _throws("Invalid register");
    }
  }

  /**
    @brief Register not complement
    @param r Register of operation
  **/
  void nots(const Register r)
  {
    auto ub = _ubyte(r);
    switch (r) {
      case al:
      case cl:
      case dl:
      case bl:
      case ah:
      case ch:
      case dh:
      case bh:
        *this << '\xF6' << _ubyte('\xD0' + ub - _ubyte(al));
        break;

      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\xF7' << _ubyte('\xD0' + ub - _ubyte(ax));
        break;

      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << '\xF7' << _ubyte('\xD0' + ub - _ubyte(eax));
        break;

#ifndef __X86_ARCH__
      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x41' << '\xF6' << _ubyte('\xD0' + ub - _ubyte(r8b));
        break;

      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41' << '\xF7' << _ubyte('\xD0' + ub - _ubyte(r8w));
        break;

      case r8d:
      case r9d:
      case r10d:
      case r11d:
      case r12d:
      case r13d:
      case r14d:
      case r15d:
        *this << '\x41' << '\xF7' << _ubyte('\xD0' + ub - _ubyte(r8d));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xF7' << _ubyte('\xD0' + ub - _ubyte(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xF7' << _ubyte('\xD0' + ub - _ubyte(r8));
        break;
#endif
      default:
        _throws("Invalid register");
    }
  }

private:
  uintptr_t _address;  //!< Points to firt byte of memory that will be patched
  uintptr_t _offset;   //!< Points to last byte written
  Data      _original; //!< Original data object
  Data      _payload;  //!< Payload data object
  size_t    _maxRead;  //!< Maximum amount of bytes to read

  /**
    @brief  operator<<
    @tparam T      Type of data to be written
    @param  p      Reference to patch object
    @param  value  Value to be written into memory
    @retval Patch& Reference to modified patch object
  **/
  template<typename T>
  inline friend Patch& _PutObject(Patch& p, const T& value)
  {
    Read(p._offset, p._original, sizeof(value), p._maxRead);
    p._payload.PushObject<T>(value);
    WriteObject<T>(p._offset, value);
    p._offset += sizeof(value);
    return p;
  }

  inline friend Patch& operator<<(Patch& p, const uint8_t& value)
  {
    return _PutObject(p, value);
  }

  inline friend Patch& operator<<(Patch& p, const int8_t& value)
  {
    return p << static_cast<uint8_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const char& value)
  {
    return p << static_cast<uint8_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint16_t& value)
  {
    return _PutObject(p, value);
  }

  inline friend Patch& operator<<(Patch& p, const int16_t& value)
  {
    return p << static_cast<uint16_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint32_t& value)
  {
    return _PutObject(p, value);
  }

  inline friend Patch& operator<<(Patch& p, const int32_t& value)
  {
    return p << static_cast<uint32_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint64_t& value)
  {
    return _PutObject(p, value);
  }

  inline friend Patch& operator<<(Patch& p, const int64_t& value)
  {
    return p << static_cast<uint64_t>(value);
  }
};

}
