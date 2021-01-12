#pragma once

#include "Base.h"

namespace scriptwrapper
{

namespace data
{

enum class Opcode : BYTE
{
  JO = 0x70,
  JNO,
  JC,
  JNC,
  JZ,
  JNZ,
  JBE,
  JNBE,
  JS,
  JNS,
  JP,
  JNP,
  JL,
  JNL,
  JLE,
  JNLE,
  NOP = 0x90
};

template<typename T>
class List : private std::list<T>
{
public:
  List& operator+=(const T& value)
  {
    this->push_back(value);
    return this;
  }

  List& operator-=(const T& value)
  {
    this->remove(value);
    return this;
  }

  void operator()(void (*iterator)(const T&))
  {
    std::for_each(begin(), end(), iterator);
  }
};

}

}
