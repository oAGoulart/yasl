#pragma once

#include "memory.h"

static void PrintCurrentProcess()
{
  Memory::Process p;

  Memory::Module m = p.GetBaseModule();
  cout << string_narrow(m.GetImageFilename());
  cout << " at " << m.GetBaseAddress().ToValue() << endl;
  cout << "Entry point: " << m.GetEntryPoint().ToValue() << endl;
}

void ProcessTest()
{
  PrintCurrentProcess();
}
