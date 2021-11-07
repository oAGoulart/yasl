/**
  @brief     PEFormat submodule
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
#include "protection.h"
#include "pointer.h"

namespace Memory
{

struct IMAGE_DOS_HEADER {
  uint8_t  e_magic[2];
  uint16_t e_cblp;
  uint16_t e_cp;
  uint16_t e_crlc;
  uint16_t e_cparhdr;
  uint16_t e_minalloc;
  uint16_t e_maxalloc;
  uint16_t e_ss;
  uint16_t e_sp;
  uint16_t e_csum;
  uint16_t e_ip;
  uint16_t e_cs;
  uint16_t e_lfarlc;
  uint16_t e_ovno;
  uint16_t e_res1[4];
  uint16_t e_oemid;
  uint16_t e_oeminfo;
  uint16_t e_res2[10];
  uint32_t e_lfanew;
};

using dosheader_t = IMAGE_DOS_HEADER;
using ntheaders_t = IMAGE_NT_HEADERS;
using meminfo_t = MEMORY_BASIC_INFORMATION;

class PEFormat {
public:
  PEFormat(const Data& signature) :
    baseAddress_(0u), dosHeader_(0u), ntHeaders_(0u), dosSignature_(signature)
  {
    baseAddress_ = FindBaseAddress_(dosSignature_);
    if (baseAddress_ == -1)
      _throws("Could not find PE data on any virtual memory section");

    dosHeader_ = baseAddress_.ToVoid();
    ntHeaders_ = baseAddress_ + dosHeader_.ToObject<dosheader_t>()->e_lfanew;
  }

  constexpr Pointer& GetBaseAddress() noexcept
  {
    return baseAddress_;
  }

  const Pointer GetEntryPoint() noexcept
  {
    return FindDynamicAddress(ntHeaders_.ToObject<ntheaders_t>()->OptionalHeader.AddressOfEntryPoint, true);
  }

  const Pointer FindDynamicAddress(const uintptr_t& staticAddress, const bool isRva = false) noexcept
  {
    auto addr = (isRva) ? staticAddress + staticBase_ : staticAddress;
    return (baseAddress_ == staticBase_) ? addr : baseAddress_ + (addr - staticBase_);
  }

private:
#ifndef __X86__
  const uintptr_t staticBase_ = 0x140000000u;
#else
  const uintptr_t staticBase_ = 0x400000u;
#endif

  Pointer baseAddress_;
  Pointer dosHeader_;
  Pointer ntHeaders_;
  Data    dosSignature_;

  static Pointer FindBaseAddress_(const Data& signature)
  {
    auto mi = make_unique<meminfo_t>();
    VirtualQuery(0u, &*mi, sizeof(*mi));

    Pointer currAddr = 0u;
    do {
      Protection protection(mi->AllocationBase, mi->RegionSize);
      currAddr += mi->RegionSize;
      if (protection.GetOldMode() == PAGE_NOACCESS)
        continue;

      if (mi->AllocationBase == mi->BaseAddress &&
          (mi->AllocationProtect | PAGE_EXECUTE_READ)) {
        size_t count = 0u;
        for (size_t i = 0u; i < signature.Size(); ++i, ++count) {
          if (signature[i] != *(reinterpret_cast<ubyte_t*>(mi->AllocationBase) + count)) {
            count = -1;
            break;
          }
        }
        if (count != -1)
          return mi->AllocationBase;
      }
    } while (VirtualQuery(currAddr.ToVoid(), &*mi, sizeof(*mi)));
    return -1;
  }
};

}
