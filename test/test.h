#pragma once

// force debug mode
#ifdef NDEBUG
#undef NDEBUG
#endif

#include "base.h"

#include <iostream>
#include <string>
#include <assert.h>
#include "config_test.h"
#include "process_test.h"

static void _InitCli();
static void _TerminateCli(int code);
