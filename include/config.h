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

    auto configFileSize = file_size(filename);
    if (configFileSize >= _CONFIG_SIZE_MAX)
      throw runtime_error(_STRCAT(__FUNCSIG__, "\tConfig file size is bigger than allowed"));

    const auto bufferSize = static_cast<size_t>(configFileSize);
    auto buffer = make_unique<wchar_t[]>(bufferSize + 1);
    _ReadLua(&file, &buffer[0], bufferSize);
    buffer[bufferSize] = L'\0'; // make sure buffer is null terminated
    file.close();

    _ParseLuaEntries(&buffer[0]);
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
    @param file   Pointer to input file stream
    @param buffer Pointer to input buffer
    @param size   Length of buffer
  **/
  void _ReadLua(wifstream* file, wchar_t* buffer, const size_t size)
  {
    const wchar_t _LUA_COMMENT_START = L'[';
    const wchar_t _LUA_COMMENT_END = L']';
    const wchar_t _LUA_COMMENT_LINE = L'-';

    wchar_t wch;
    size_t n = 0;
    while (file->get(wch)) {
      // ignore Lua comments
      if (wch == _LUA_COMMENT_LINE && file->peek() == _LUA_COMMENT_LINE) {
        file->seekg(3, ios_base::cur); // skip comment start
        if (file->get() == _LUA_COMMENT_START && file->peek() == _LUA_COMMENT_START) {
          for (auto c = file->get(); c != EOF; c = file->get()) {
            if (c == _LUA_COMMENT_END && file->peek() == _LUA_COMMENT_END) {
              file->seekg(3, ios_base::cur); // skip comment end
              break;
            }
            if (c == _LUA_COMMENT_START && file->peek() == _LUA_COMMENT_START)
              throw runtime_error(_STRCAT(__FUNCSIG__, "\tFound wrapped multi-line comments"));
          }
        }
        else
          while (file->get() != '\n' && file->peek() != EOF); // go to next line
        continue;
      }
      if (n >= size)
        break;

      buffer[n] = wch;
      ++n;
    }
  };

  /**
    @brief Parse Lua entries and maps found on file
    @param buffer Pointer to buffer used with @c _ReadLua
  **/
  void _ParseLuaEntries(wchar_t* buffer)
  {
    const wchar_t* _ENTRY_ASSIGN = L"=";
    const wchar_t* _LUA_TABLE_START = L"{";
    const wchar_t* _LUA_TABLE_END = L"}";
    const wchar_t* _STR_INVALID = L"=()[]|!@#$%&*";
    const wchar_t* _STR_DELIMITER = L"\x20\x2C\t\n\r";

    wchar_t* token = nullptr;
    wchar_t* nextToken = nullptr;
    bool isParsingTable = false;
    token = wcstok_s(&buffer[0], _STR_DELIMITER, &nextToken);
    while (token != nullptr) {
      wstring str = token; // expects: key name or end of table
      if (isParsingTable && str == _LUA_TABLE_END) {
        isParsingTable = false;
        continue;
      }
      if (str.find_first_of(_STR_INVALID) != wstring::npos)
        break;

      token = wcstok_s(nullptr, _STR_DELIMITER, &nextToken);
      if (token == nullptr)
        break;
      wstring eql = token; // expects: = operator

      token = wcstok_s(nullptr, _STR_DELIMITER, &nextToken);
      if (token == nullptr)
        break;
      wstring key = token; // expects: key value or start of table

      if (eql == _ENTRY_ASSIGN && key.find_first_of(_STR_INVALID) != wstring::npos)
        throw runtime_error(_STRCAT(__FUNCSIG__, "\tFound unexpected characters"));

      if (isParsingTable)
        _maps.back().Emplace(str, key);
      else if (key == _LUA_TABLE_START) {
        _maps.emplace_back(str);
        isParsingTable = true;
      }
      else
        _entries.emplace_back(str, key);

      token = wcstok_s(nullptr, _STR_DELIMITER, &nextToken);
    }
  };
};
