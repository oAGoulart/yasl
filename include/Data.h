#pragma once

#include "Base.h"

namespace scriptwrapper
{

namespace data
{

/* TODO: Document this enum */
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

/* TODO: Document this class */
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
