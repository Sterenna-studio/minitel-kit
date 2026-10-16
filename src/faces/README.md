# faces/ (optionnel, décision 6 de docs/CONCEPTION.md)

Moteur d'yeux (transposition de web-eye-animation, MIT, CyberAgentAILab ; licence dans `NOTICE`) et
six visages G1 (Classic, Robot, Alien, Eyes, Zyra, Zyra Eyes), à extraire de
`LemegetonEyes.h` et `Galerie.h` de Minitel_Sigil.

Entrées du moteur, sans rien de Sigil : émotion (identifiants de
`lemegeton-core/canon/emotions.json`), regard, clignement, posture durable
(neutre, fatigué, concentré, calme). Sortie : un `G1Canvas`.

Avant de dessiner : `docs/PIEGES.md`, section 3, et canon/VIDEOTEX.md.
