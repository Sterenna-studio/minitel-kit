#pragma once
#include <Arduino.h>
#include <FS.h>

// Format MFA1 : animations texte 40 x 20.
//
//   en-tete (8 octets) : 'M' 'F' 'A' '1', nombre d'images (16 bits),
//                        couleur 1..7, 0
//   puis par image     : duree en ms (16 bits), 800 caracteres (40 x 20)
//
// A porter depuis AsciiPlayer.h de Minitel_Sigil (meme format que
// AsciiService.h de Minitel_Face_Autonomous). Lecture seule, sur un File
// quelconque : stockage, import et interface restent a l'appelant.
// Fixer l'ordre des octets des champs 16 bits d'apres le code au portage.

namespace minitelkit {

class Mfa1Reader {
 public:
  bool open(File file);
  uint16_t frames() const;
  uint8_t color() const;
  // Lit l'image suivante dans `image` (20 x 40) ; false en fin de fichier.
  bool next(char image[20][40], uint16_t& durationMs);
  bool rewind();
};

}  // namespace minitelkit
