#pragma once

#include "config.h"

static void _ValidConfig()
{
  ConfigFile cfg("./validConfig.lua");

  // alphanumeric values do not need string quotes
  _asserts(cfg.FindEntry(L"SomeName") == L"Alice", "");
  // numeric values can be cast into proper type
  _asserts(stof(cfg.FindEntry(L"SomeValue")) == 10.5f, "");
  // double quotes string
  _asserts(path(cfg.FindEntry(L"SomePath")) == L"./this/potato/is/mine.pdf", "");
  // integer value
  _asserts(stoi(cfg.FindEntry(L"Potato", L"Temp")) == 260, "");
  // single quotes string
  _asserts(cfg.FindEntry(L"Potato", L"Color") == L"yellow", "");
  // boolean value
  _asserts(cfg.FindEntry(L"Potato", L"some_bool") == L"true", "");
  // multiline string
  _asserts(cfg.FindEntry(L"some_multiline_string") == L"This is a very big\n\n  multiline string\n\n", "");
}

static void _SketchyConfig()
{
  ConfigFile cfg("./sketchyConfig.lua"); // weirdly formatted file, but still valid

  _asserts(cfg.FindEntry(L"name") == L"bob", "");
  _asserts(stof(cfg.FindEntry(L"size")) == 0.55050f, "");
  _asserts(path(cfg.FindEntry(L"theop")) == L"isop", "");
  _asserts(cfg.FindEntry(L"map", L"isgreen") == L"false", "");
  _asserts(cfg.FindEntry(L"map", L"___private") == L"yousee", "");
  _asserts(stoi(cfg.FindEntry(L"x")) == 10, "");
  _asserts(stoi(cfg.FindEntry(L"y")) == 53, "");
  _asserts(stoi(cfg.FindEntry(L"pos", L"x")) == -23, "");
  _asserts(stoi(cfg.FindEntry(L"pos", L"y")) == 0, "");
}

void ConfigTest()
{
  _ValidConfig();
  _SketchyConfig();
}
