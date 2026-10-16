#pragma once
#include <Arduino.h>

// Outils qui ont trouve les pannes de la liaison (docs/TESTS.md).
// A porter depuis Minitel_Sigil.ino.

namespace minitelkit {
namespace diag {

// Fil d'Ariane en memoire RTC non initialisee : survit a un redemarrage
// logiciel ou au chien de garde. step = derniere etape de loop() atteinte.
void breadcrumb(uint32_t step);
void breadcrumbKey(uint32_t key);
void breadcrumbMode(uint32_t mode);
// Au demarrage : vrai si un fil valide existe (et le rend), puis le remet a 0.
bool previousBreadcrumb(uint32_t& step, uint32_t& key, uint32_t& mode);

// heap_caps_check_integrity_all ; ne signale que la PREMIERE corruption.
bool checkHeap(const char* after, uint32_t detail = 0);

}  // namespace diag
}  // namespace minitelkit
