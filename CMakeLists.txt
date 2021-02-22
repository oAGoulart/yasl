# ScriptWrapper CMake building file
cmake_minimum_required(VERSION 3.10)

project(scriptwrapper VERSION 0.0.5 LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(lib)

option(SW_BUILD_UNIT_TESTS "Build ScriptWrapper's unit tests" ON)
option(SW_BUILD_DOCS "Build ScriptWrapper's documentation" OFF)

if (SW_BUILD_DOCS)
  set(DOXYGEN_SKIP_DOT TRUE)
  find_package(Doxygen)
endif()

if (DOXYGEN_FOUND AND SW_BUILD_DOCS)
  add_subdirectory(docs)
endif()

if (SW_BUILD_UNIT_TESTS)
  add_subdirectory(tests)
endif()
