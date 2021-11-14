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
    filename_(filename), name_(name), version_(version)
  {
    file_.open(filename_);
#ifdef __MARKDOWN_EXTEND__
    file_ << L"\t**" << name_ << L"** " << version_ << L" status output..." << _wcrlf
          << _wcrlf << flush;
#else
    file_ << L'\t' << name_ << L' ' << version_ << L" status output..." << _wcrlf
          << _wcrlf << flush;
#endif
  };

  /**
    @brief Status object destructor
  **/
  ~Status()
  {
    file_.close();
  };

  /**
    @brief Prints message to log file
    @param msg String to be print
  **/
  void LogMessage(const wstring& msg)
  {
    auto stamp = system_clock::now();
    auto time = system_clock::to_time_t(stamp);
    auto buffer = make_unique<wchar_t[]>(_staticSize);
    if (_wctime64_s(&buffer[0], _staticSize, &time))
      _throws("Could not convert time into string");

    auto length = wcsnlen_s(&buffer[0], _staticSize);
    buffer[length - 1] = L'\0'; // remove new line char

#ifdef __MARKDOWN_EXTEND__
    file_ << L"_[" << buffer << L"]_ **" << name_ << L"**: " << msg << _wcrlf << flush;
#else
    file_ << L'[' << buffer << L"] " << name_ << L": " << msg << _wcrlf << flush;
#endif
  };

  static bool GetSystemInfo(wstring& outStr)
  {

    outStr += L"System metrics and configuration settings:" _wcrlf;
    outStr += L"\tBoot mode: ";
    auto displayCount = GetSystemMetrics(SM_CLEANBOOT);
    outStr += (displayCount) ? L"Fail-safe boot" _wcrlf : L"Normal boot" _wcrlf;
    outStr += L"\tDisplay count: ";
    outStr += to_wstring(GetSystemMetrics(SM_CMONITORS)) + _wcrlf;
    auto mouseButtons = GetSystemMetrics(SM_CMOUSEBUTTONS);
    outStr += L"\tMouse buttons: ";
    outStr += (mouseButtons) ? to_wstring(mouseButtons) + _wcrlf : L"Mouse not found" _wcrlf;
    outStr += L"\tScreen width: ";
    outStr += to_wstring(GetSystemMetrics(SM_CXSCREEN)) + _wcrlf;
    outStr += L"\tScreen height: ";
    outStr += to_wstring(GetSystemMetrics(SM_CYSCREEN)) + _wcrlf;

    if (displayCount > 1) {
      outStr += L"\tVirtual screen width: ";
      outStr += to_wstring(GetSystemMetrics(SM_CXVIRTUALSCREEN)) + _wcrlf;
      outStr += L"\tVirtual screen height: ";
      outStr += to_wstring(GetSystemMetrics(SM_CYVIRTUALSCREEN)) + _wcrlf;
    }

    auto hasNetwork = GetSystemMetrics(SM_NETWORK);
    outStr += L"\tNetwork: ";
    outStr += (hasNetwork & 1) ? L"Available" _wcrlf : L"Not available" _wcrlf;

    return true;
  }

private:
  path      filename_;  //!< Path to log file
  wofstream file_;      //!< File output stream
  wstring   name_;      //!< Project name
  wstring   version_;   //!< Project version
};
