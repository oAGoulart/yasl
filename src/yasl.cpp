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
    Fatal(e);
    return false;
  }
  return true;
}

/**
  @brief Initialize hooks and callbacks
**/
static void Start()
{
  SetUnhandledExceptionFilter(Status::CustomSEHFilter); // TODO: nop setter

  auto pe = make_unique<Memory::PEFormat>(nullptr);

  // hook entry point with single use trampoline
  _trampoline = new Memory::Trampoline<int>(pe->GetEntry(), 1);
  _trampoline->before += &Hook;
}

/**
  @brief Terminate trampoline
**/
static void End()
{
  delete _trampoline;
}

/**
  @brief YASL object constructor
**/
YASL::YASL()
{
  _status = make_unique<Status>(_LOG_FILE, _PROJECT_NAME, _PROJECT_VERSION);
  _LoadConfig();
  _LoadScripts();
}

/**
  @brief YASL object destructor
**/
YASL::~YASL()
{
  _status->LogMessage(L"Returning to entry point");
}

/**
  @brief Run loaded scripts
**/
void YASL::Run()
{
  _status->LogMessage(L"Running scripts");
}

/**
  @brief  Check if file extension is supported
  @param  filename Filename
  @retval          Is file extension supported?
**/
bool YASL::_IsFileExtSupported(const path& filename) const
{
  auto ext = filename.extension();
  for (auto str = _supportedExt.begin(); str != _supportedExt.end(); ++str) {
    if (*str == ext)
      return true;
  }
  return false;
}

/**
  @brief Load configuration from file
**/
void YASL::_LoadConfig()
{
  auto config = make_unique<ConfigFile>(_CONFIG_FILE);
  _status->LogMessage(L"Loading and parsing configuration file");

  auto cfgStr = config->FindEntry(L"MainName");
  if (cfgStr.empty())
    cfgStr = L"StartScript";
  _mainName = cfgStr;

  cfgStr = config->FindEntry(L"FileExtensions");
  if (cfgStr.empty())
    cfgStr = L".asi;.dff;";

  const wchar_t* extDiv = L";";
  auto index = cfgStr.find_first_of(extDiv);
  size_t lastIndex = 0;
  while (index != wstring::npos) {
    _supportedExt.push_back(cfgStr.substr(lastIndex, index));
    lastIndex = index;
    index = cfgStr.find_first_of(extDiv, index + 1);
  }

  cfgStr = config->FindEntry(L"ScriptsFolder");
  if (cfgStr.empty())
    cfgStr = L"./";
  _scriptsFolder = cfgStr;
}

/**
  @brief Load scripts into memory
**/
void YASL::_LoadScripts()
{
  _status->LogMessage(L"Loading scripts into memory");
  for (recursive_directory_iterator next(_scriptsFolder), end; next != end; ++next) {
    if (!next->is_directory()) {
      if (_IsFileExtSupported(next->path())) {
        auto name = next->path().native();

        auto script = LoadLibraryW(name.c_str());
        if (script != nullptr) {
          size_t count;
          auto buffer = make_unique<char[]>(_STATIC_BUFF_SIZE);
          wcstombs_s(&count, &buffer[0], _STATIC_BUFF_SIZE, _mainName.c_str(), _STATIC_BUFF_SIZE);

          auto func = GetProcAddress(script, &buffer[0]);
          if (func != nullptr) {
            _scripts.emplace_back(script, func, next->path().filename());
            _status->LogMessage(name.append(L"\t->\tScript loaded successfully"));
            continue;
          }
        }
        _status->LogMessage(name.append(L"\t->\tWARNING: could not load script"));
      }
    }
  }
}

/**
  @brief Generate fatal error file
  @param e Error
**/
static void Fatal(const exception& e)
{
  hfile_t* tmp;
  freopen_s(&tmp, "./yaslFatal.md", "w", stderr);
  fprintf_s(tmp, "Fatal:\n\t%s\n", e.what());
  fclose(tmp);
}

/**
  @brief Dummy empty function
**/
void Dummy()
{
}

/**
  @brief Procedure hook function
**/
int Hook()
{
  auto yasl = make_unique<YASL>();
  yasl->Run();
  return 0;
}
