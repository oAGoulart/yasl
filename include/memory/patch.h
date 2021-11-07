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
#include "pointer.h"
#include "data.h"
#include "peformat.h"

namespace Memory
{

/**
  @enum  Memory::Register
  @brief List of Assembly x86-32 and x86-64 registers
  @note  Members value is used to calcule some opcodes binary value, changing
         this @c enum order will cause undefined behavior
**/
enum class Register : ubyte_t {
  // byte sized registers
  al, cl, dl, bl, ah, ch, dh, bh,
#ifndef __X86__
  spl, bpl, sil, dil,
  r8b, r9b, r10b, r11b, r12b, r13b, r14b, r15b,
#endif
  // word sized registers
  ax, cx, dx, bx, sp, bp, si, di,
#ifndef __X86__
  r8w, r9w, r10w, r11w, r12w, r13w, r14w, r15w,
#endif
  // dword sized registers
  eax, ecx, edx, ebx, esp, ebp, esi, edi,
#ifndef __X86__
  r8d, r9d, r10d, r11d, r12d, r13d, r14d, r15d,
  // qword sized registers
  rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi,
  r8, r9, r10, r11, r12, r13, r14, r15,
#endif
  noreg = 255u // marks end of registers
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

  Patch(const PEFormat& image, const Pointer& ptr) :
    image_(image), ptr_(ptr), offset_(ptr), original_({}), payload_({})
  {
    if (ptr_ < image_.GetBaseAddress())
      _throws("Patch address is less than base address");
  }

  /**
    @brief  Gets size of patch
    @retval size_t Number of bytes written into memory
  **/
  const size_t GetCount() const noexcept
  {
    return offset_.ToValue() - ptr_.ToValue();
  }

  /**
    @brief  Get original data
    @retval Data& Original Data object, which contains the buffer storing
            the data replaced by the patch
  **/
  constexpr Data& GetOriginal() noexcept
  {
    return original_;
  }

  /**
    @brief  Get payload data
    @retval Data& Current payload Data object with patch values
  **/
  constexpr Data& GetPayload() noexcept
  {
    return payload_;
  }

  /**
    @brief Restore original data into memory
    @note  Calling this function will clear the payload buffer
           and subsequent patches will start from @c _address
  **/
  void Restore()
  {
    if (original_.Size()) {
      Write(ptr_, original_, original_.Size());
      offset_ = ptr_;
      payload_.Clear();
    }
  };

  void mov(const Register r, const uint8_t value)
  {
    auto ub = static_cast<ubyte_t>(r);
    switch (r) {
      case al:
      case cl:
      case dl:
      case bl:
      case ah:
      case ch:
      case dh:
      case bh:
        *this << static_cast<ubyte_t>('\xB0' + ub - static_cast<ubyte_t>(al));
        break;

#ifndef __X86__
      case spl:
      case bpl:
      case sil:
      case dil:
        *this << '\x40'
              << static_cast<ubyte_t>('\xB4' + ub - static_cast<ubyte_t>(spl));
        break;

      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x41'
              << static_cast<ubyte_t>('\xB0' + ub - static_cast<ubyte_t>(r8b));
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
    auto ub = static_cast<ubyte_t>(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66'
              << static_cast<ubyte_t>('\xB8' + ub - static_cast<ubyte_t>(ax));
        break;

#ifndef __X86__
      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41'
              << static_cast<ubyte_t>('\xB8' + ub - static_cast<ubyte_t>(r8w));
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
    auto ub = static_cast<ubyte_t>(r);
    switch (r) {
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << static_cast<ubyte_t>('\xB8' + ub - static_cast<ubyte_t>(eax));
        break;

#ifndef __X86__
      case r8d:
      case r9d:
      case r10d:
      case r11d:
      case r12d:
      case r13d:
      case r14d:
      case r15d:
        *this << '\x41'
              << static_cast<ubyte_t>('\xB8' + ub - static_cast<ubyte_t>(r8d));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xC7'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xC7'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(r8));
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
    auto ub = static_cast<ubyte_t>(r);
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
        *this << '\x88'
              << '\x0D' + static_cast<ubyte_t>((ub - static_cast<ubyte_t>(cl)) * 8u)
              << '\x12';
        break;

#ifndef __X86__
      case spl:
      case bpl:
      case sil:
      case dil:
        *this << '\x40' << '\x88'
              << '\x24' + static_cast<ubyte_t>((ub - static_cast<ubyte_t>(spl)) * 8u)
              << '\x25';
        break;

      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x44' << '\x88'
              << '\x04' + static_cast<ubyte_t>((ub - static_cast<ubyte_t>(r8b)) * 8u)
              << '\x25';
        break;
#endif
      case eax:
#ifdef __X86__
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
        *this << '\x89'
              << static_cast<ubyte_t>('\x0D' + (ub - static_cast<ubyte_t>(eax)) * 8u);
        break;

#ifndef __X86__
      case r8d:
      case r9d:
      case r10d:
      case r11d:
      case r12d:
      case r13d:
      case r14d:
      case r15d:
        *this << '\x41'
              << static_cast<ubyte_t>('\xB8' + ub - static_cast<ubyte_t>(r8d));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xC7'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xC7'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(r8));
        break;
#endif
      default:
        _throws("Invalid register");
    }
    *this << address;
  }

#ifndef __X86__
  /**
    @brief Move value into register

    Move unsigned 64bit value into qword register

    @param r     Register destination
    @param value Value to be moved
  **/
  void movabs(const Register r, const uint64_t value)
  {
    auto ub = static_cast<ubyte_t>(r);
    switch (r) {
      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48'
              << static_cast<ubyte_t>('\xB8' + ub - static_cast<ubyte_t>(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49'
              << static_cast<ubyte_t>('\xB8' + ub - static_cast<ubyte_t>(r8));
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
    auto ub = static_cast<ubyte_t>(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66'
              << static_cast<ubyte_t>('\x50' + ub - static_cast<ubyte_t>(ax));
        break;

#ifdef __X86__
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << static_cast<ubyte_t>('\x50' + ub - static_cast<ubyte_t>(eax));
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
        *this << '\x66' << '\x41'
              << static_cast<ubyte_t>('\x50' + ub - static_cast<ubyte_t>(r8w));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << static_cast<ubyte_t>('\x50' + ub - static_cast<ubyte_t>(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41'
              << static_cast<ubyte_t>('\x50' + ub - static_cast<ubyte_t>(r8));
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
    auto ub = static_cast<ubyte_t>(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66'
              << static_cast<ubyte_t>('\x58' + ub - static_cast<ubyte_t>(ax));
        break;

#ifdef __X86__
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << static_cast<ubyte_t>('\x58' + ub - static_cast<ubyte_t>(eax));
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
        *this << '\x66' << '\x41'
              << static_cast<ubyte_t>('\x58' + ub - static_cast<ubyte_t>(r8w));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << static_cast<ubyte_t>('\x58' + ub - static_cast<ubyte_t>(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41'
              << static_cast<ubyte_t>('\x58' + ub - static_cast<ubyte_t>(r8));
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
    *this << static_cast<int32_t>(address - (offset_ + 4u));
  }

  /**
    @brief Absolute jump into address
    @param r Register containing destination address
  **/
  void jmp(const Register r)
  {
    auto ub = static_cast<ubyte_t>(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\xFF'
              << static_cast<ubyte_t>('\xE0' + ub - static_cast<ubyte_t>(ax));
        break;

#ifdef __X86__
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << '\xFF'
              << static_cast<ubyte_t>('\xE0' + ub - static_cast<ubyte_t>(eax));
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
        *this << '\x66' << '\x41' << '\xFF'
              << static_cast<ubyte_t>('\xE0' + ub - static_cast<ubyte_t>(r8w));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\xFF'
              << static_cast<ubyte_t>('\xE0' + ub - static_cast<ubyte_t>(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << '\xFF'
              << static_cast<ubyte_t>('\xE0' + ub - static_cast<ubyte_t>(r8));
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
    *this << static_cast<int32_t>(address - (offset_ + 4u));
  }

  /**
    @brief Absolute call function
    @param r Register containing function address
  **/
  void call(const Register r)
  {
    auto ub = static_cast<ubyte_t>(r);
    switch (r) {
      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\xFF'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(ax));
        break;

#ifdef __X86__
      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << '\xFF'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(eax));
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
        *this << '\x66' << '\x41' << '\xFF'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(r8w));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\xFF'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x41' << '\xFF'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(r8));
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
    Read(offset_, original_, count);
    Fill(offset_, '\x90', count);
    offset_ += count;
  }

  /**
    @brief Return from call/jump
    @param isFar Is return far?
  **/
  void ret(const bool isFar = false)
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
  void intr(const ubyte_t code)
  {
    *this << '\xCD' << code;
  }

  /**
    @brief Jump if equal
    @param address Destination address
  **/
  void je(const uintptr_t address)
  {
    *this << '\x0F' << '\x84' << static_cast<int32_t>(address - (offset_ + 4u));
  }
  void (Patch::*jz)(const uintptr_t) = &Patch::je;  //!< Jump if zero

  /**
    @brief Jump if not equal
    @param address Destination address
  **/
  void jne(const uintptr_t address)
  {
    *this << '\x0F' << '\x85' << static_cast<int32_t>(address - (offset_ + 4u));
  }
  void (Patch::*jnz)(const uintptr_t) = &Patch::jne;  //!< Jump if not zero

  /**
    @brief Jump if greater
    @param address Destination address
  **/
  void jg(const uintptr_t address)
  {
    *this << '\x0F' << '\x8F' << static_cast<int32_t>(address - (offset_ + 4u));
  }
  void (Patch::*jnle)(const uintptr_t) = &Patch::jg;  //!< Jump if not less or equal

  /**
    @brief Jump if greater or equal
    @param address Destination address
  **/
  void jge(const uintptr_t address)
  {
    *this << '\x0F' << '\x8D' << static_cast<int32_t>(address - (offset_ + 4u));
  }
  void (Patch::*jnl)(const uintptr_t) = &Patch::jge;  //!< Jump if not less

  /**
    @brief Jump if less
    @param address Destination address
  **/
  void jl(const uintptr_t address)
  {
    *this << '\x0F' << '\x8C' << static_cast<int32_t>(address - (offset_ + 4u));
  }
  void (Patch::*jnge)(const uintptr_t) = &Patch::jl;  //!< Jump if not greater or equal

  /**
    @brief Jump less or equal
    @param address Destination address
  **/
  void jle(const uintptr_t address)
  {
    *this << '\x0F' << '\x8E' << static_cast<int32_t>(address - (offset_ + 4u));
  }
  void (Patch::*jng)(const uintptr_t) = &Patch::jle;  //!< Jump not greater

  /**
    @brief Increase register value by one
    @param r Register to be increased
  **/
  void inc(const Register r)
  {
    auto ub = static_cast<ubyte_t>(r);
    switch (r) {
      case al:
      case cl:
      case dl:
      case bl:
      case ah:
      case ch:
      case dh:
      case bh:
        *this << '\xFE'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(al));
        break;

      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
#ifdef __X86__
        *this << '\x66'
              << static_cast<ubyte_t>('\x40' + ub - static_cast<ubyte_t>(ax));
#else
        *this << '\x66' << '\xFF'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(ax));
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
#ifdef __X86__
        *this << static_cast<ubyte_t>('\x40' + ub - static_cast<ubyte_t>(eax));
#else
        *this << '\xFF'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(ax));
#endif
        break;

#ifndef __X86__
      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x41' << '\xFE'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(r8b));
        break;

      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41' << '\xFF'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(r8w));
        break;

      case r8d:
      case r9d:
      case r10d:
      case r11d:
      case r12d:
      case r13d:
      case r14d:
      case r15d:
        *this << '\x41' << '\xFF'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(r8d));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xFF'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xFF'
              << static_cast<ubyte_t>('\xC0' + ub - static_cast<ubyte_t>(r8));
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
    auto ub = static_cast<ubyte_t>(r);
    switch (r) {
      case al:
      case cl:
      case dl:
      case bl:
      case ah:
      case ch:
      case dh:
      case bh:
        *this << '\xFE'
              << static_cast<ubyte_t>('\xC8' + ub - static_cast<ubyte_t>(al));
        break;

      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
#ifdef __X86__
        *this << '\x66'
              << static_cast<ubyte_t>('\x48' + ub - static_cast<ubyte_t>(ax));
#else
        *this << '\x66' << '\xFF'
              << static_cast<ubyte_t>('\xC8' + ub - static_cast<ubyte_t>(ax));
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
#ifdef __X86__
        *this << static_cast<ubyte_t>('\x48' + ub - static_cast<ubyte_t>(eax));
#else
        *this << '\xFF'
              << static_cast<ubyte_t>('\xC8' + ub - static_cast<ubyte_t>(eax));
#endif
        break;

#ifndef __X86__
      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x41' << '\xFE'
              << static_cast<ubyte_t>('\xC8' + ub - static_cast<ubyte_t>(r8b));
        break;

      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41' << '\xFF'
              << static_cast<ubyte_t>('\xC8' + ub - static_cast<ubyte_t>(r8w));
        break;

      case r8d:
      case r9d:
      case r10d:
      case r11d:
      case r12d:
      case r13d:
      case r14d:
      case r15d:
        *this << '\x41' << '\xFF'
              << static_cast<ubyte_t>('\xC8' + ub - static_cast<ubyte_t>(r8d));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xFF'
              << static_cast<ubyte_t>('\xC8' + ub - static_cast<ubyte_t>(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xFF'
              << static_cast<ubyte_t>('\xC8' + ub - static_cast<ubyte_t>(r8));
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
    auto ub = static_cast<ubyte_t>(r);
    switch (r) {
      case al:
      case cl:
      case dl:
      case bl:
      case ah:
      case ch:
      case dh:
      case bh:
        *this << '\xF6'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(al));
        break;

      case ax:
      case cx:
      case dx:
      case bx:
      case sp:
      case bp:
      case si:
      case di:
        *this << '\x66' << '\xF7'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(ax));
        break;

      case eax:
      case ecx:
      case edx:
      case ebx:
      case esp:
      case ebp:
      case esi:
      case edi:
        *this << '\xF7'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(eax));
        break;

#ifndef __X86__
      case r8b:
      case r9b:
      case r10b:
      case r11b:
      case r12b:
      case r13b:
      case r14b:
      case r15b:
        *this << '\x41' << '\xF6'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(r8b));
        break;

      case r8w:
      case r9w:
      case r10w:
      case r11w:
      case r12w:
      case r13w:
      case r14w:
      case r15w:
        *this << '\x66' << '\x41' << '\xF7'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(r8w));
        break;

      case r8d:
      case r9d:
      case r10d:
      case r11d:
      case r12d:
      case r13d:
      case r14d:
      case r15d:
        *this << '\x41' << '\xF7'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(r8d));
        break;

      case rax:
      case rcx:
      case rdx:
      case rbx:
      case rsp:
      case rbp:
      case rsi:
      case rdi:
        *this << '\x48' << '\xF7'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(rax));
        break;

      case r8:
      case r9:
      case r10:
      case r11:
      case r12:
      case r13:
      case r14:
      case r15:
        *this << '\x49' << '\xF7'
              << static_cast<ubyte_t>('\xD0' + ub - static_cast<ubyte_t>(r8));
        break;
#endif
      default:
        _throws("Invalid register");
    }
  }

private:
  PEFormat image_;
  Pointer  ptr_;
  Pointer  offset_;
  Data     original_;
  Data     payload_;

  /**
    @brief  operator<<
    @tparam T      Type of data to be written
    @param  p      Reference to patch object
    @param  value  Value to be written into memory
    @retval Patch& Reference to modified patch object
  **/
  template<typename T>
  inline friend Patch& PutObject_(Patch& p, const T& value)
  {
    Read(p.offset_, p.original_, sizeof(value));
    p.payload_.PushObject<T>(value);
    WriteObject<T>(p.offset_, value);
    p.offset_ += sizeof(value);
    return p;
  }

  inline friend Patch& operator<<(Patch& p, const uint8_t& value)
  {
    return PutObject_(p, value);
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
    return PutObject_(p, value);
  }

  inline friend Patch& operator<<(Patch& p, const int16_t& value)
  {
    return p << static_cast<uint16_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint32_t& value)
  {
    return PutObject_(p, value);
  }

  inline friend Patch& operator<<(Patch& p, const int32_t& value)
  {
    return p << static_cast<uint32_t>(value);
  }

  inline friend Patch& operator<<(Patch& p, const uint64_t& value)
  {
    return PutObject_(p, value);
  }

  inline friend Patch& operator<<(Patch& p, const int64_t& value)
  {
    return p << static_cast<uint64_t>(value);
  }
};

}
