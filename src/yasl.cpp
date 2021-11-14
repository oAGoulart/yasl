/**
  @file      yasl.cpp
  @brief     Main source file
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
#include "pch.h"
#include "yasl.h"

/**
  @brief  DllMain function callback
  @param         Unused
  @param  reason Reason for DLL attachment
  @param         Unused
  @retval        Was DLL attached?
**/
lbool_t WINAPI DllMain(hmodule_t, ulong_t reason, pvoid_t)
{
  try {
    if (reason == DLL_PROCESS_ATTACH)
      Start();
    else if (reason == DLL_PROCESS_DETACH)
      End();
  }
  catch (const exception& e) {
    _fatal(e);
    return false;
  }
  return true;
}

/**
  @brief Initialize hooks and callbacks
**/
static void Start()
{
  hYasl_ = new YASL();
}

/**
  @brief Terminate trampoline
**/
static void End()
{
  delete hYasl_;
}

/**
  @brief YASL object constructor
**/
YASL::YASL()
{
  status_ = make_unique<Status>(logFile_, projectName_, projectVersion_);
  LoadConfig_();
  LoadScripts_();

  Memory::Process p;
  Memory::Module m = p.GetBaseModule();
  trampoline_ = make_unique<Memory::Trampoline<int>>(m.GetEntryPoint(), 1);
  // TODO: add _RunScripts() function
  trampoline_->before += &Hook; // TODO: hook does not need to run on trampoline, only scripts
}

/**
  @brief YASL object destructor
**/
YASL::~YASL()
{
  status_->LogMessage(L"Returning to entry point");
}

// TODO: move to Script module
bool YASL::IsFileExtSupported_(const path& filename) const
{
  auto ext = filename.extension();
  for (auto str = supportedExt_.begin(); str != supportedExt_.end(); ++str) {
    if (*str == ext)
      return true;
  }
  return false;
}

/**
  @brief Load configuration from file
**/
void YASL::LoadConfig_()
{
  auto config = make_unique<ConfigFile>(configFile_);
  status_->LogMessage(L"Loading and parsing configuration file");

  auto cfgStr = config->FindEntry(L"MainName");
  if (cfgStr.empty())
    cfgStr = L"StartScript";
  mainName_ = cfgStr;

  cfgStr = config->FindEntry(L"FileExtensions");
  if (cfgStr.empty())
    cfgStr = L".asi;.dff;";

  const wchar_t* extDiv = L";";
  auto index = cfgStr.find_first_of(extDiv);
  size_t lastIndex = 0;
  while (index != wstring::npos) {
    supportedExt_.push_back(cfgStr.substr(lastIndex, index));
    lastIndex = index;
    index = cfgStr.find_first_of(extDiv, index + 1);
  }

  cfgStr = config->FindEntry(L"ScriptsFolder");
  if (cfgStr.empty())
    cfgStr = L"./";
  scriptsFolder_ = cfgStr;
}

// TODO: move to Script module
void YASL::LoadScripts_()
{
  status_->LogMessage(L"Loading scripts into memory");
  for (recursive_directory_iterator next(scriptsFolder_), end; next != end; ++next) {
    if (!next->is_directory()) {
      if (IsFileExtSupported_(next->path())) {
        auto name = next->path().native();

        auto script = LoadLibraryW(name.c_str());
        if (script != nullptr) {
          size_t count;
          auto buffer = make_unique<char[]>(_staticSize);
          wcstombs_s(&count, &buffer[0], _staticSize, mainName_.c_str(), _staticSize);

          auto func = GetProcAddress(script, &buffer[0]);
          if (func != nullptr) {
            scripts_.emplace_back(script, func, next->path().filename());
            status_->LogMessage(name.append(L"\t->\tScript loaded successfully"));
            continue;
          }
        }
        status_->LogMessage(name.append(L"\t->\tWARNING: could not load script"));
      }
    }
  }
}

/**
  @brief Dummy empty function
**/
void Dummy()
{
}

// NOTE: no longer needed, will hook from YASL constructor
int Hook()
{
  MessageBox(nullptr, L"hook has been moved", L"no crashorino", 0);
  return 0;
}
