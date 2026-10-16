#pragma once
#include <stdint.h>

// Octets de mosaique G1 et verifications. Memes regles que g1.js de
// minitel-face et que canon/VIDEOTEX.md de lemegeton-core.
//
//   0x01 | 0x02    tiers haut
//   0x04 | 0x08    tiers milieu
//   0x10 | 0x40    tiers bas        0x20 toujours present
//
// Les noms disent ce qui est ALLUME : 0x70 est le tiers bas, pas « le haut ».

namespace minitelkit {
namespace g1 {

constexpr uint8_t FULL = 0x7F;
constexpr uint8_t EMPTY = 0x20;
constexpr uint8_t HIGH_THIRD = 0x23;
constexpr uint8_t MID_THIRD = 0x2C;
constexpr uint8_t LOW_THIRD = 0x70;
constexpr uint8_t UPPER_TWO = 0x2F;  // deux tiers hauts

// Plages valides : 0x20-0x3F et 0x60-0x7F.
constexpr bool valid(uint8_t b) { return (b >= 0x20 && b <= 0x3F) || (b >= 0x60 && b <= 0x7F); }

// Six bits dans l'ordre haut-gauche, haut-droit, milieu-gauche,
// milieu-droit, bas-gauche, bas-droit.
constexpr uint8_t sextants(uint8_t b) { return (b & 0x1F) | ((b & 0x40) >> 1); }
constexpr uint8_t fromSextants(uint8_t s) { return 0x20 | (s & 0x1F) | ((s & 0x20) << 1); }

// Miroir gauche-droite : une forme se decrit par sa moitie gauche, la droite
// est calculee (pas d'embouts depareilles).
uint8_t mirror(uint8_t b);

}  // namespace g1
}  // namespace minitelkit
