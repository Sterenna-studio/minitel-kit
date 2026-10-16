#pragma once
#include <Arduino.h>
#include <Minitel1B_Hard.h>

// Grille texte 24 x 40 a emission differentielle. A extraire de
// Experiences.h (xpFrame / xpPrev) de Minitel_Sigil. Exemples : aquarium,
// pluie de caracteres, page 40 x 20 recue.

namespace minitelkit {

class TextGrid {
 public:
  void clear();
  void put(uint8_t row, int col, const char* text);
  bool flush(Minitel& m, HardwareSerial& port, size_t headroom = 96);
  void invalidate();
};

}  // namespace minitelkit
