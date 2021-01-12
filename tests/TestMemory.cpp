#include "../include/Memory.h"

using namespace scriptwrapper;

int main() {
  memory::Patch<int, int> getPotato(0x6050607);

  getPotato.before += [](int potato) {
    return 40;
  };
  getPotato.after += [](int potato) {
    return 50;
  };
}
