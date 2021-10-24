/**
  @brief     Config module
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

/**
  @class ConfigFile
  @brief Object used to store configure file information
**/
class ConfigFile {
public:
  /**
    @brief ConfigFile object constructor
    @param filename Path to configure file
  **/
  ConfigFile(const path& filename) : _filename(filename)
  {
    wifstream file;
    file.open(_filename);

    wstring buffer;
    _ReadLua(file, buffer);
    file.close();
    _ParseLuaEntries(buffer);
  };

  /**
    @brief  Search for entry on configuration
    @param  name Entry name
    @retval      Key value or empty string if not found
  **/
  wstring FindEntry(const wstring name) noexcept
  {
    for (auto entry = _entries.begin(); entry != _entries.end(); ++entry) {
      if (entry->GetName() == name)
        return entry->GetKey();
    }
    return L"";
  };

  /**
    @brief  Search for entry on maps configuration
    @param  map  Map name
    @param  name Entry name
    @retval      Key value or empty string if not found
  **/
  wstring FindEntry(const wstring map, const wstring name) noexcept
  {
    for (auto item = _maps.begin(); item != _maps.end(); ++item) {
      if (item->GetName() == map)
        return item->FindEntry(name);
    }
    return L"";
  };

private:
  /**
    @class Entry
    @brief Object to store entry data
  **/
  class Entry {
  public:
    /**
      @brief Entry object constructor
      @param name Entry name
      @param key  Key value
    **/
    Entry(const wstring& name, const wstring& key) :
      _name(name), _key(key)
    {
    };

    /**
      @brief  Gets entry name
      @retval Entry name
    **/
    wstring& GetName() noexcept
    {
      return _name;
    };

    /**
      @brief  Gets key value
      @retval Key value
    **/
    wstring& GetKey() noexcept
    {
      return _key;
    };

  private:
    wstring _name; //!< Entry name
    wstring _key;  //!< Key value
  };

  /**
    @class Map
    @brief Object used to store maps from configuration
  **/
  class Map {
  public:
    /**
      @brief Map object constructor
      @param name Map name
    **/
    Map(const wstring& name) : _name(name)
    {
    };

    /**
      @brief Emplace entry into map
      @param name Entry name
      @param key  Key value
    **/
    void Emplace(const wstring& name, const wstring& key)
    {
      _entries.emplace_back(name, key);
    };

    /**
      @brief  Search for entry inside map
      @param  name Entry name
      @retval      Entry key or empty string if not found
    **/
    wstring FindEntry(const wstring name) noexcept
    {
      for (auto entry = _entries.begin(); entry != _entries.end(); ++entry) {
        if (entry->GetName() == name)
          return entry->GetKey();
      }
      return L"";
    };

    /**
      @brief  Gets map name
      @retval Map name
    **/
    wstring& GetName() noexcept
    {
      return _name;
    };

  private:
    wstring     _name;    //!< Map name
    list<Entry> _entries; //!< List of entries inside map
  };

  path        _filename; //!< Path to configuration file
  list<Entry> _entries;  //!< List of entries on configuration
  list<Map>   _maps;     //!< List of maps on configuration

  /**
    @brief Read Lua file for config entries and maps
    @param file   Input file stream
    @param buffer String to store file data
  **/
  void _ReadLua(wifstream& file, wstring& buffer)
  {
    wchar_t buf[4];
    size_t count = 0;

    while (file.read(&buf[0], 4)) {
      if (buf[1] == buf[0]) {
        if (buf[0] == L'-') { // "--" single line comment
          count += 4;
          while (file.get(buf[0]) && buf[0] != L'\n')
            ++count;
          file.seekg(count);
          continue;
        }
        else if (buf[0] == L'[' && buf[2] == L'-' && buf[3] == buf[2]) { // "[[--" multiline comment
          while (file.read(&buf[0], 4)) {
            if (buf[1] == buf[0] && buf[3] == buf[2]) {
              if (buf[0] == L'[' && buf[2] == L'-') // "[[--"
                _throws("Found nested multiline comments");
              if (buf[0] == L'-' && buf[2] == L']') { // "--]]"
                count += 4;
                file.seekg(count);
                break;
              }
            }
            file.seekg(++count);
          }
          continue;
        }
      }
      buffer += buf[0];
      file.seekg(++count);
    }
    buffer += buf[0];
    buffer += buf[1];
    buffer += buf[2];
  };

  /**
    @brief Parse Lua entries and maps found on file
    @param buffer Wide string buffer
  **/
  void _ParseLuaEntries(wstring& buffer)
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

    for (auto wch = buffer.begin(); wch != buffer.end(); ++wch) {
      if (!state.parsingEntry && !state.parsingKey && iswspace(*wch))
        continue;

      if (state.foundTable && !state.parsingTable) {
        _maps.emplace_back(entry);
        entry.clear();

        state.foundTable = false;
        state.parsingTable = true;
        state.foundEntry = false;
        state.needsEntry = true;
      }

      if (state.needsEntry) {
        if (*wch == L'}') { // end of table
          state.parsingTable = false;
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
        auto next = ((++tmp) != buffer.end()) ? *tmp : L'\0';

        if (*wch == L'{' && !state.parsingTable) { // start of table
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

        if (state.parsingTable)
          _maps.back().Emplace(entry, key);
        else
          _entries.emplace_back(entry, key);

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
  };
};
