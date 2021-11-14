#pragma once

#include "base.h"
#include "pointer.h"
#include "data.h"

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

class Module {
public:
  Module() : baseAddress_(0u), dosHeader_(0u), ntHeaders_(0u)
  {
  }

  Module(const wstring imageFile, const Pointer base) :
    imageFile_(imageFile), baseAddress_(base), dosHeader_(0u), ntHeaders_(0u)
  {
    dosHeader_ = baseAddress_.ToVoid();
    ntHeaders_ = baseAddress_ + dosHeader_.ToObject<dosheader_t>()->e_lfanew;
    isDll_ = ntHeaders_.ToObject<ntheaders_t>()->FileHeader.Characteristics & IMAGE_FILE_DLL;
  }

  constexpr wstring& GetImageFilename() noexcept
  {
    return imageFile_;
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
    auto base = (isDll_) ? dllStaticBase_ : staticBase_;
    auto addr = (isRva) ? staticAddress + base : staticAddress;
    return (baseAddress_ == base) ? addr : baseAddress_ + (addr - base);
  }

  void operator=(const Module& eq)
  {
    imageFile_ = eq.imageFile_;
    baseAddress_ = eq.baseAddress_;
    dosHeader_ = baseAddress_.ToVoid();
    ntHeaders_ = baseAddress_ + dosHeader_.ToObject<dosheader_t>()->e_lfanew;
    isDll_ = ntHeaders_.ToObject<ntheaders_t>()->FileHeader.Characteristics & IMAGE_FILE_DLL;
  }

private:
#ifndef __X86__
  const uintptr_t staticBase_ = 0x140000000u;
  const uintptr_t dllStaticBase_ = 0x180000000u;
#else
  const uintptr_t staticBase_ = 0x400000u;
  const uintptr_t dllStaticBase_ = 0x10000000u;
#endif

  wstring imageFile_;
  Pointer baseAddress_;
  Pointer dosHeader_;
  Pointer ntHeaders_;
  bool    isDll_;
};

class Process {
public:
  Process()
  {
    imageFile_.resize(_staticSize);
    GetModuleFileNameW(nullptr, imageFile_.data(), _staticSize);

    EnumerateLoadedModules();
  }

  constexpr Module& GetBaseModule() noexcept
  {
    return base_;
  }

  void EnumerateLoadedModules()
  {
    auto mi = make_unique<meminfo_t>();
    VirtualQuery(0u, &*mi, sizeof(*mi));

    Pointer currAddr = 0u;
    do {
      Protection protection(mi->AllocationBase, mi->RegionSize);
      currAddr += mi->RegionSize;

      if (protection.GetOldMode() == PAGE_NOACCESS)
        continue;
      if (mi->AllocationBase == mi->BaseAddress && mi->Protect == PAGE_READONLY) {
        wstring name;
        name.resize(_staticSize);
        GetModuleFileNameW(reinterpret_cast<hmodule_t>(mi->BaseAddress), name.data(), _staticSize);
        modules_.emplace_back(name, mi->BaseAddress);

        if (imageFile_ == name)
          base_ = modules_.back();
      }
    } while (VirtualQuery(currAddr.ToVoid(), &*mi, sizeof(*mi)));

    if (base_.GetBaseAddress().ToVoid() == nullptr)
      _throws("Could not find process base module");
  }

private:
  wstring        imageFile_;
  Module         base_;
  vector<Module> modules_;
};

}
