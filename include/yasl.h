/**
  @brief     Main module
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
#include "config.h"
#include "script.h"
#include "memory.h"
#include "status.h"

class YASL {
public:
  YASL();
  ~YASL();
  void Run();

private:
  list<wstring>      _supportedExt;  //!< List of supported file extensions
  wstring            _mainName;      //!< Name of scripts main function
  path               _scriptsFolder; //!< Path to scripts folder
  list<Script>       _scripts;       //!< List of scripts found
  unique_ptr<Status> _status;        //!< Pointer to status object

  const path _CONFIG_FILE = L"./yasl.lua";  //!< Configure file path
  const path _LOG_FILE = L"./yaslLog.md";   //!< Log file path
  const wstring _PROJECT_NAME = L"YASL";    //!< Project name
  const wstring _PROJECT_VERSION = L"v0.1"; //!< Project version

  bool _IsFileExtSupported(const path& filename) const;
  void _LoadConfig();
  void _LoadScripts();
};

static void Start();
static void End();
void Dummy();
static void Hook();
static void Fatal(const exception& e);
