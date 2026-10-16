# minitel-kit

Piloter un vrai **Minitel 1B** depuis un **ESP32** sans jamais bloquer la
boucle, et tout ce qu'on a appris en le faisant.

*Drive a real French Minitel 1B from an ESP32 without ever stalling the main
loop, plus the hard-won notes on the terminal. Documentation in French.*

> **État : squelette.** Les interfaces de `src/` sont posées, aucun code n'est
> encore porté. Voir [docs/CONCEPTION.md](docs/CONCEPTION.md).

## Contenu

| Dossier | Rôle |
| --- | --- |
| `src/` | Bibliothèque Arduino `MinitelKit` : lien et vitesse bornés, clavier non bloquant, texte replié en ASCII, mosaïques G1 à émission différentielle, lecteur d'animations MFA1, diagnostics |
| `examples/` | Sonde du terminal, codes clavier, console de test |
| `docs/TERMINAL_M1B.md` | Fiche du Minitel 1B (Alcatel M1B) : vitesses, 7E1, protocole utile, codes clavier |
| `docs/PIEGES.md` | Les pièges déjà payés, datés et sourcés |
| `docs/TESTS.md` | Procédures de test sur le vrai terminal |

## Matériel visé

- Minitel 1B (40 colonnes Vidéotex, 300/1200/4800 bauds) ;
- dongle iodeo Minitel-ESP32 (ESP32-WROOM-32E), core esp32 2.0.17,
  **PSRAM désactivée** (elle reprend GPIO 16/17, la liaison Minitel).

## Dépendances

- [Minitel1B_Hard](https://github.com/eserandour/Minitel1B_Hard) (obligatoire) ;
- [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets) (pont
  WebSocket seulement, à venir).

## La règle du kit

Aucune attente sans limite de temps, aucune attente de plus de ~100 ms sans
rendre la main. Plusieurs fonctions de `Minitel1B_Hard` ne la respectent pas ;
le kit les remplace (voir [docs/PIEGES.md](docs/PIEGES.md)).

## Projets liés

- [lemegeton-core](https://github.com/Sterenna-studio/lemegeton-core) : canon
  du personnage et règles de dessin G1 (`canon/VIDEOTEX.md`) ;
- [minitel-musee](https://github.com/Sterenna-studio/minitel-musee) : Minitel
  3D et simulation Vidéotex dans le navigateur ;
- [3615-gateways](https://github.com/Sterenna-studio/3615-gateways) : la
  passerelle 3615 côté serveur.

## Licence

À décider (voir la conception).
