/**
  @brief     Status module
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

#include <chrono>
#include <ctime>

#pragma comment(lib, "Dbghelp.lib")

using namespace chrono;

/**
  @class Status
  @brief Object used to store project status
**/
class Status {
public:
  /**
    @brief Status object constructor
    @param filename Path to log file
    @param name     Project name
    @param version  Project version
  **/
  Status(const path& filename, const wstring& name, const wstring& version) :
    _filename(filename), _name(name), _version(version)
  {
    _file.open(_filename);
#ifdef __MARKDOWN_EXTEND__
    _file << L"\t**" << _name << L"** " << _version << L" status output..." << L"\n\n" << flush;
#else
    _file << L'\t' << _name << L' ' << _version << L" status output..." << L"\n\n" << flush;
#endif
  };

  /**
    @brief Status object destructor
  **/
  ~Status()
  {
    _file.close();
  };

  /**
    @brief Prints message to log file
    @param msg String to be print
  **/
  void LogMessage(const wstring& msg)
  {
    auto stamp = system_clock::now();
    auto time = system_clock::to_time_t(stamp);
    auto buffer = make_unique<wchar_t[]>(_STATIC_BUFF_SIZE);
    if (_wctime64_s(&buffer[0], _STATIC_BUFF_SIZE, &time))
      _throws("Could not convert time into string");

    auto length = wcsnlen_s(&buffer[0], _STATIC_BUFF_SIZE);
    buffer[length - 1] = L'\0'; // remove new line char

#ifdef __MARKDOWN_EXTEND__
    _file << L"_[" << buffer << L"]_ **" << _name << L"**: " << msg << endl << flush;
#else
    _file << L'[' << buffer << L"] " << _name << L": " << msg << endl << flush;
#endif
  };

  /**
    @brief  Custom SEH filter callback function
    @param  exceptionInfo Pointer to exception info
    @retval               Exception status
  **/
  static long_t WINAPI CustomSEHFilter(exception_t* exceptionInfo)
  {
    wchar_t name[MAX_PATH];
    wchar_t* item;
    if (GetModuleFileNameW(GetModuleHandle(nullptr), name, MAX_PATH)) {
      item = wcsrchr(name, L'\\');
      *item = L'\0';
      ++item;
    }
    else
      wcscpy_s(name, L"err.unknown");

    auto file = CreateFileW(L"miniDump.md", GENERIC_WRITE, FILE_SHARE_WRITE,
                            nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
      minidump_t miniDump;
      memset(&miniDump, 0, sizeof(miniDump));
      miniDump.ThreadId = GetCurrentThreadId();
      miniDump.ExceptionPointers = exceptionInfo;
      miniDump.ClientPointers = true;
      MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                        file, MiniDumpWithDataSegs, &miniDump, nullptr, nullptr);
      CloseHandle(file);
    }

    ShowCursor(true);
    auto wnd = FindWindowW(0, L"");
    SetForegroundWindow(wnd);

    return EXCEPTION_CONTINUE_SEARCH;
  };

private:
  path      _filename;  //!< Path to log file
  wofstream _file;      //!< File output stream
  wstring   _name;      //!< Project name
  wstring   _version;   //!< Project version
};
