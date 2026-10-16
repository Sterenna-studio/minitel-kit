#pragma once

// MinitelKit : piloter un Minitel 1B depuis un ESP32 sans jamais bloquer la
// boucle. Squelette : les interfaces sont posees, le code sera porte depuis
// les firmwares un module a la fois (voir docs/CONCEPTION.md).
//
// Regle du kit : aucune attente sans limite de temps, aucune attente de plus
// de ~100 ms sans rendre la main (docs/PIEGES.md).

#include <Minitel1B_Hard.h>

#include "MinitelLink.h"
#include "MinitelKeys.h"
#include "MinitelText.h"
#include "G1.h"
#include "G1Canvas.h"
#include "TextGrid.h"
#include "Mfa1.h"
#include "Diagnostics.h"
