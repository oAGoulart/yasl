/**
  @brief     Settings module
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
#include "settings/entry.h"

namespace Settings
{

class Config {
public:
  Config(const path& filename) :
    filename_(filename), file_(), head_(L"__g__"), buffer_()
  {
    file_.open(filename_);
    Read_();
    file_.close();
    Parse_();
  }

  Entry& operator[](const wstring& name) noexcept
  {
    return head_[name];
  }

private:
  path      filename_;
  wifstream file_;
  Entry     head_;
  wstring   buffer_;

  void Read_()
  {
    wchar_t buf[4] = {};
    size_t count = 0;

    while (file_.read(&buf[0], 4)) {
      if (buf[1] == buf[0]) {
        if (buf[0] == L'-') { // "--" single line comment
          count += 4;
          if (buf[2] != L'\n' && buf[3] != L'\n') {
            for (; file_.get(buf[0]) && buf[0] != L'\n'; ++count);
            file_.seekg(count);
          }
          buf[1] = L'\x20';
          buf[2] = L'\x20';
          continue;
        }
        else if (buf[0] == L'[' && buf[2] == L'-' && buf[3] == buf[2]) { // "[[--" multiline comment
          while (file_.read(&buf[0], 4)) {
            if (buf[1] == buf[0] && buf[3] == buf[2]) {
              if (buf[0] == L'[' && buf[2] == L'-') // "[[--"
                _throws("Found nested multiline comments");
              if (buf[0] == L'-' && buf[2] == L']') { // "--]]"
                count += 4;
                file_.seekg(count);
                break;
              }
            }
            file_.seekg(++count);
          }
          continue;
        }
      }
      buffer_ += buf[0];
      file_.seekg(++count);
    }
    buffer_ += buf[0];
    buffer_ += buf[1];
    buffer_ += buf[2];
  }

  void Parse_()
  {
    struct {
      bool needsEntry = true;
      bool foundEntry = false;
      bool parsingEntry = false;
      bool needsKey = false;
      bool foundKey = false;
      bool parsingKey = false;
      bool parsingString = false;
      bool foundTable = false;
      bool parsingTable = false;
    } state;
    wstring entry, key;
    list<Entry*> currentTable;
    currentTable.push_back(&head_);

    for (auto wch = buffer_.begin(); wch != buffer_.end(); ++wch) {
      if (!state.parsingEntry && !state.parsingKey && iswspace(*wch))
        continue;

      if (state.foundTable && !state.parsingTable) {
        currentTable.back()->Add({ entry });
        currentTable.push_back(&currentTable.back()->GetTail());
        entry.clear();

        state.foundTable = false;
        state.parsingTable = true;
        state.foundEntry = false;
        state.needsEntry = true;
      }

      if (state.needsEntry) {
        if (*wch == L'}') { // end of table
          state.parsingTable = false;
          currentTable.pop_back();

          if (currentTable.empty())
            _throws("Unexpected end of table");
          continue;
        }

        if (iswalpha(*wch) || *wch == L'_') { // restrict to valid names
          entry += *wch;
          state.parsingEntry = true;
          continue;
        }
        else {
          state.needsEntry = false;
          state.parsingEntry = false;
          state.foundEntry = true;
        }
      }
      else if (state.needsKey) {
        auto tmp = wch;
        auto next = ((++tmp) != buffer_.end()) ? *tmp : L'\0';

        if (*wch == L'{') { // start of table
          state.needsKey = false;
          state.foundTable = true;
          continue;
        }
        else if (*wch == L'\'' || *wch == L'\"') { // start/end of string
          state.parsingString = !state.parsingString;
          continue;
        }
        else if ((*wch == L'[' || *wch == L']') && *wch == next) { // start/end of string
          ++wch;
          state.parsingString = !state.parsingString;
          continue;
        }

        if (state.parsingString || iswalnum(*wch) || *wch == L'.' || *wch == L'-') {
          key += *wch;
          state.parsingKey = true;
          continue;
        }
        else {
          state.needsKey = false;
          state.parsingKey = false;
          state.foundKey = true;
        }
      }

      if (state.foundEntry && state.foundKey) {
        state.foundEntry = false;
        state.foundKey = false;
        state.needsEntry = true;

        currentTable.back()->Add({ { entry, key } });
        entry.clear();
        key.clear();
        continue;
      }
      else if (state.foundEntry && !state.needsKey && !iswspace(*wch)) {
        if (*wch == L'=') {
          state.needsKey = true;
          continue;
        }
        else
          _throws("Expected operator=");
      }

      if (iswpunct(*wch) && *wch != L',')
        _throws("Unexpected character found while parsing");
    }
  }
};

}
