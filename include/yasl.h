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
#include "settings.h"
#include "script.h"
#include "memory.h"
#include "status.h"

#define \
_fatal(e) \
{ \
  hfile_t* tmp; \
  freopen_s(&tmp, "./yaslFatal.md", "w", stderr); \
  fprintf_s(tmp, "FATAL ERROR\n\t%s\n", e.what()); \
  fclose(tmp); \
}

class YASL {
public:
  YASL();
  ~YASL();

private:
  list<Script>       scripts_;       // TODO: create script pool object
  unique_ptr<Status> status_;        //!< Pointer to status object
  unique_ptr<Settings::Config> config_;
  unique_ptr<Memory::Trampoline<int>> trampoline_;

  const path configFile_ = L"./yasl.lua";    //!< Configure file path
  const path logFile_ = L"./yaslLog.md";     //!< Log file path
  const wstring projectName_ = L"YASL";      //!< Project name
  const wstring projectVersion_ = L"v0.8.0"; //!< Project version
};

static YASL* hYasl_; // need this from program start until termination

static void Start();
static void End();
extern "C" void Dummy();
int Run();
