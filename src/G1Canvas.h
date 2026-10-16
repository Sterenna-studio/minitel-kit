#pragma once
#include <Arduino.h>
#include <Minitel1B_Hard.h>

// Tampon de cellules G1 (octet + couleur par cellule) et emission
// differentielle : seules les cellules qui changent partent sur la liaison.
// A extraire du tampon commun de LemegetonEyes.h / Galerie.h (Minitel_Sigil).
//
// Chaque segment emis se termine par G0 + espace (attribut « en espace ») :
// ce caractere tampon gomme la cellule qui suit (docs/PIEGES.md, section 3).

namespace minitelkit {

class G1Canvas {
 public:
  G1Canvas(uint8_t firstRow, uint8_t rows);  // rangees 1..24, 40 colonnes

  void clear();
  void put(uint8_t row, uint8_t col, uint8_t color, uint8_t octet);
  // Une rangee entiere : une region possede ses lignes, bords compris.
  void fillRow(uint8_t row, uint8_t color, uint8_t octet);

  // Emet les cellules changees si au moins `headroom` octets sont libres
  // dans le tampon d'emission ; false si l'image a ete differee.
  bool flush(Minitel& m, HardwareSerial& port, size_t headroom = 96);
  void invalidate();  // prochaine image = image complete
};

}  // namespace minitelkit
