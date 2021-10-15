/*
  Copyright (c) 2021 Augusto Goulart
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
*/
#include "yasl.h"

lbool_t WINAPI DllMain(hmodule_t mod, ulong_t reason, pvoid_t)
{
  try {
    if (reason == DLL_PROCESS_ATTACH)
      Start();
    else if (reason == DLL_PROCESS_DETACH)
      End(); // not always reached on parent termination
  }
  catch (const exception& e) {
    hfile_t* tmp;
    freopen_s(&tmp, "./yaslFatal.md", "w", stderr);
    fprintf_s(tmp, "Fatal:\n\t%s\n", e.what());
    fclose(tmp);
    return FALSE;
  }
  return TRUE;
}

void Start()
{
  SetUnhandledExceptionFilter(Status::CustomSEHFilter); // TODO: nop setter

  auto buffer = make_unique<char[]>(MAX_PATH); // MapAndLoad doesn't support wchar_t
  if (!GetModuleFileNameA(NULL, &buffer[0], MAX_PATH))
    throw runtime_error(_STRCAT(__FUNCSIG__, "\tUnable to find process filename"));

  LOADED_IMAGE image;
  if (!MapAndLoad(&buffer[0], NULL, &image, FALSE, TRUE))
    throw runtime_error(_STRCAT(__FUNCSIG__, "\tCould not map process binary file"));

  uint32_t entry = image.FileHeader->OptionalHeader.AddressOfEntryPoint + BASE_ADDRESS;
  UnMapAndLoad(&image);

  // hook entry point with single use trampoline
}

void End()
{
}

YASL::YASL()
{
  _status = make_unique<Status>(_LOG_FILE, _PROJECT_NAME, _PROJECT_VERSION);
  _status->LogMessage(L"Redirecting entry point call");
  _LoadConfig();
  _LoadScripts();
}

YASL::~YASL()
{
  _status->LogMessage(L"Returning to entry point");
}

void YASL::Run()
{
  _status->LogMessage(L"Running scripts");
}

bool YASL::_IsFileExtSupported(const path& filename) const
{
  auto ext = filename.extension();
  for (auto str = _supportedExt.begin(); str != _supportedExt.end(); ++str) {
    if (*str == ext)
      return true;
  }
  return false;
}

void YASL::_LoadConfig()
{
  auto config = make_unique<ConfigFile>(_CONFIG_FILE);
  _status->LogMessage(L"Loading and parsing configuration file");

  // MainName
  auto cfgStr = config->FindEntry(L"MainName");
  if (cfgStr.empty())
    cfgStr = L"StartScript";
  _mainName = cfgStr;

  // FileExtensions
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

  // ScriptsFolder
  cfgStr = config->FindEntry(L"ScriptsFolder");
  if (cfgStr.empty())
    cfgStr = L"./";
  _scriptsFolder = cfgStr;
}

void YASL::_LoadScripts()
{
  _status->LogMessage(L"Loading scripts into memory");
  for (recursive_directory_iterator next(_scriptsFolder), end; next != end; ++next) {
    if (!next->is_directory()) {
      if (_IsFileExtSupported(next->path())) {
        auto script = LoadLibraryW(&next->path().native()[0]);
        if (script == nullptr)
          continue;

        size_t count;
        auto buffer = make_unique<char[]>(_STATIC_BUFF_SIZE);
        wcstombs_s(&count, &buffer[0], _STATIC_BUFF_SIZE,
                   _mainName.c_str(), _STATIC_BUFF_SIZE);
        auto func = GetProcAddress(script, &buffer[0]);
        if (func != nullptr)
          _scripts.emplace_back(script, func, next->path().filename());
      }
    }
  }
}

void Dummy()
{
}

void Hook()
{
  auto yasl = make_unique<YASL>();
  yasl->Run();
}
