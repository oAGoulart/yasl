/**
  @brief     Settings entry submodule
  @author    Augusto Goulart
  @date      18.04.2022
  @copyright   Copyright (c) 2022 Augusto Goulart
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

namespace Settings
{

class Entry {
public:
  Entry(const wstring& name, const wstring& key) :
    name_(name), key_(key), isTable_(false)
  {
  }

  Entry(const wstring& name) :
    name_(name), table_({}), isTable_(true)
  {
  }

  Entry(const Entry& entry) :
    name_(entry.name_), isTable_(entry.isTable_)
  {
    if (isTable_)
      table_.insert(table_.begin(), entry.table_.begin(), entry.table_.end());
    else
      key_ = entry.key_;
  }

  ~Entry()
  {
    name_.~basic_string();
    if (isTable_)
      table_.~list();
    else
      key_.~basic_string();
  };

  void Add(std::initializer_list<Entry> l) {
    table_.insert(table_.end(), l.begin(), l.end());
  }

  wstring& GetName() noexcept
  {
    return name_;
  }

  const wstring GetRaw() noexcept
  {
    return (isTable_) ? L"" : key_;
  }

  Entry& GetTail() noexcept
  {
    return table_.back();
  }

  Entry& operator[](const wstring& name) noexcept
  {
    if (isTable_) {
      for (auto entry = table_.begin(); entry != table_.end(); ++entry) {
        if (entry->GetName() == name)
          return *entry;
      }
    }
    return *this;
  }

private:
  wstring name_;
  union {
    wstring     key_;
    list<Entry> table_;
  };
  bool isTable_;
};

}
