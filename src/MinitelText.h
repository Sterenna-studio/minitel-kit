#pragma once
#include <Arduino.h>
#include <Minitel1B_Hard.h>

// Texte G0 sur 40 colonnes. A porter depuis Screen.h de Minitel_Sigil.

namespace minitelkit {

constexpr uint8_t SCREEN_COLS = 40;

// UTF-8 courant (accents, guillemets et apostrophes typographiques) replie en
// ASCII affichable ; autre sequence multi-octets -> '?'.
String asciiFold(const String& in);

// Un caractere isole, sans toucher au reste de la rangee.
void writeCharAt(Minitel& m, uint8_t col, uint8_t row, char c, uint8_t color = CARACTERE_BLANC);

// Une rangee complete (1..24), le reste de la ligne efface.
void writeRow(Minitel& m, uint8_t row, const String& text, uint8_t color = CARACTERE_BLANC);

// Coupe `text` en lignes de `width` colonnes, aux espaces si possible.
// `truncated` dit si du texte a ete perdu (a annoncer, jamais en silence).
uint8_t wrapText(const String& text, String lines[], uint8_t maxLines, bool& truncated,
                 uint8_t width = SCREEN_COLS);

}  // namespace minitelkit
