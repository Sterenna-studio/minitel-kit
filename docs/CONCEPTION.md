# Conception de minitel-kit

Brouillon du 2026-10-16. Statut : **squelette publié, aucun code porté**. Les décisions encore ouvertes sont en fin de document.

## Le besoin

Le savoir-faire Minitel est aujourd'hui éparpillé dans des dépôts privés,
mêlé à ce qui est propre à Sigil (la mesh domestique) :

- le firmware actif `Minitel_Sigil` (privé) ;
- son prédécesseur `Minitel_Face_Autonomous` (privé, reflashable) ;
- `minitel-face` (privé) : serveur Node, rendu G1 en JavaScript ;
- les règles de dessin G1 dans `lemegeton-core/canon/VIDEOTEX.md` (public).

Chaque nouveau projet Minitel recopie un fichier, puis un autre, et rejoue
les mêmes pannes (attentes sans limite de la bibliothèque, embouts G1
dépareillés, vitesse de port fausse). Le but est un dépôt **public** qui
rassemble :

1. une **bibliothèque Arduino** pour ESP32 + Minitel, sans rien de Sigil ;
2. la **connaissance du terminal** : fiche du Minitel 1B, pièges payés,
   procédures de test.

Nom proposé : **`Sterenna-studio/minitel-kit`** (libre au 2026-10-16).
Nom de bibliothèque Arduino : `MinitelKit`.

## Inventaire : générique ou propre à Sigil

Légende : **K** = va dans le kit (après découplage), **E** = exemple du kit,
**S** = reste dans le firmware, **D** = seulement en documentation.

### Minitel_Sigil

| Fichier | Verdict | Ce qui part, ce qui reste |
| --- | --- | --- |
| `MinitelKeys.h` | **K** | Lecteur clavier non bloquant, mêmes codes que `getKeyCode(false)`. À découpler de `Serial2` (prendre un `Stream&`) et de `Serial.printf` (rappel de journal). `minitelTerminalReady` et la sonde vont dans le module lien. |
| `Minitel_Sigil.ino` : `interrogerVitesseMinitel`, `commandeMinitel`, `vitesseMinitelPort`, `reglerVitesseMinitelPort`, `probeMinitelTerminal` | **K** | Module « lien » : détection de vitesse bornée, commandes à acquittement borné, vitesse lue dans le matériel, écho coupé et mode page sans `echo()`/`pageMode()`. Ces fonctions sont **en cours de modification** par une autre session : on portera leur version stabilisée, pas celle d'aujourd'hui. |
| `Minitel_Sigil.ino` : fil d'Ariane RTC, `verifierTas` | **K** (module diagnostic, optionnel) | Rien de Minitel dedans, mais c'est l'outillage qui a trouvé les pannes de la liaison. Petit, sans dépendance. |
| `Minitel_Sigil.ino` : modes, `handleKey`, Wi-Fi, mDNS | **S** | Navigation et réseau propres au firmware. |
| `Screen.h` | **K** | `asciiFold` (UTF-8 → ASCII affichable), `writeCharAt`, `writeRow`, `wrapText`. Ne dépend que de l'objet `Minitel`. |
| `LemegetonEyes.h` | **K** (module « yeux ») | Moteur : timelines des dix émotions, poses, clignements, rendu en mosaïque G1 80 × 63 et tampon de cellules à diff. À découpler de `faceStyle` (global du `.ino`), de `eyesFollowSigil`, des réglages NVS et de `minitelCanRender`. Transposition de web-eye-animation (MIT) : garder la mention. Les styles 0–3 deviennent des options du moteur. |
| `Galerie.h` | **K** (module « visages ») | Les six visages G1 (Classic, Robot, Alien, Eyes, Zyra, Zyra Eyes) écrits dans le tampon commun. Seule attache à Sigil : la posture lue dans `SigilView` (dort / travaille / dégradé) ; elle devient une entrée `posture` du moteur. |
| `Experiences.h` | **E** + **K** | La grille texte 24 × 40 à diff (`xpFrame`/`xpPrev`) est un composant générique ; l'aquarium, la démo, Matrix et l'audio sont des exemples. Les réglages NVS, le sous-menu et `appMode` restent dans le firmware. |
| `AsciiPlayer.h` | **K** + **S** | Lecteur du format **MFA1** (en-tête, images 40 × 20, durées) sur un `File` quelconque : kit. Liste à l'écran, routes HTTP, page d'import : firmware (ou exemple). |
| `Korigan.h` | **K** (pont WebSocket brut) | Client Vidéotex « octets bruts » par WebSocket, avec les deux correctifs payés (pas d'en-tête `Origin`, nom `.local` résolu une seule fois). Les valeurs par défaut (hôte, port, chemin), la NVS et le nom Korigan restent dans le firmware. |
| `Bluetooth.h` | **D** | A2DP sur ESP32 : rien de Minitel. Ses pièges mémoire (réserve BLE, Wi-Fi + A2DP) vont dans `PIEGES.md`, section ESP32. |
| `Media.h` | **E** | Affichage d'une page 40 × 20 reçue : exemple de la grille texte. |
| `BootAnim.h`, `Hub.h`, `FaceScreen.h`, `Terminal.h`, `SettingsScreen.h` | **S** | Écrans du firmware (sommaire, parole, annonces, 3615 SIGIL, réglages). `Hub.h` a `writeAt`/`drawHubBar` génériques : à reprendre seulement si un second projet en a besoin. |
| `SigilLink.h`, `WebApi.h`, `WebPages.h`, `Settings.h`, `SerialConsole.h` | **S** | Lien avec le cœur, API, NVS, console. Les commandes de test `TOUCHE`, `DEBIT`, `VITESSE` sont décrites dans `docs/TESTS.md` et fournies comme **exemple** de console, sans les commandes Sigil. |

### Minitel_Face_Autonomous

| Fichier | Verdict |
| --- | --- |
| `MinitelKeys.h` | Identique à celui de Minitel_Sigil (au nom près dans un commentaire) : une seule copie, celle du kit. |
| `HubServices.h` (visages G1) | Déjà porté dans `Galerie.h` avec les embouts corrigés (`ac6bbcd`) : on part de `Galerie.h`. |
| `AsciiService.h` | Origine du format MFA1 ; même format que `AsciiPlayer.h`. |
| `docs/G1_MOSAIQUES.md`, `docs/TROUBLESHOOTING.md` | **D** : sources de `PIEGES.md` et de la fiche du terminal. |
| `Esp32Admin.h`, `DeviceManagement.h`, `BluetoothSpeakerService.h`, `OllamaChannel.h`, `AutomationService.h` | Hors kit : administration ESP32, Ollama, scènes. Rien de Minitel. |

### minitel-face (serveur Node, privé)

| Élément | Verdict |
| --- | --- |
| `src/libs/g1.js` (`mirrorG1`, `sym`, `mirrorRow`, constantes G1) | **K, phase 2** : paquet JS du kit (`js/`), même règles que le C++. |
| `src/EscOptimizer.js` (suppression des GOTO/couleurs redondants) | **K, phase 2** (`js/`). |
| `docs/minitel_display_notes.md`, `docs/display_strategy.md` | **D** : attribut « en espace », patch partiel sans CLS. |
| `docs/Minitel-ESP32-main/` (GPL-3), `docs/imgtovdt-…`, `docs/videotex-codes.pdf` | **Ne pas copier** : tiers. Lien vers les sources d'origine. |
| `LibZyra.js`, `LibClassic.js`… | Restent dans minitel-face (rendu serveur du personnage). |

## Structure proposée

```text
minitel-kit/
├── library.properties        bibliothèque Arduino « MinitelKit »
├── src/
│   ├── MinitelKit.h          en-tête parapluie
│   ├── MinitelLink.h/.cpp    vitesse bornée, commandes à acquittement, sonde
│   ├── MinitelKeys.h/.cpp    clavier non bloquant
│   ├── MinitelText.h/.cpp    asciiFold, writeRow, wrapText
│   ├── G1.h                  octets G1, miroir, vérifications
│   ├── G1Canvas.h/.cpp       tampon de cellules G1 + émission différentielle
│   ├── TextGrid.h/.cpp       grille texte 24 × 40 + émission différentielle
│   ├── Mfa1.h/.cpp           lecteur du format MFA1
│   ├── VideotexWsBridge.h    pont WebSocket brut (optionnel, arduinoWebSockets)
│   ├── Diagnostics.h         fil d'Ariane RTC, intégrité du tas
│   └── faces/                moteur d'yeux et six visages (optionnel)
├── examples/
│   ├── Sonde/                détecter le Minitel et passer à 4800 bauds
│   ├── Clavier/              afficher les codes des touches
│   ├── ConsoleDeTest/        TOUCHE, DEBIT, VITESSE, ARIANE
│   └── Yeux/                 les yeux, pilotés au clavier
├── docs/
│   ├── CONCEPTION.md         ce document
│   ├── TERMINAL_M1B.md       fiche du Minitel 1B
│   ├── PIEGES.md             pièges déjà payés, sourcés
│   ├── TESTS.md              procédures de test sur le vrai terminal
│   └── VIDEOTEX.md           (décision 5) règles G1, si elles migrent du canon
└── js/                       phase 2 : g1.js, EscOptimizer
```

### Principes de l'API

- **Aucun global imposé.** Chaque module prend ce qu'il utilise :
  `Minitel&`, `HardwareSerial&` ou `Stream&`. Pas de `Serial2` en dur, pas de
  `Serial.printf` : un rappel de journal optionnel.
- **Aucune attente sans limite.** Toute fonction qui attend le terminal a un
  délai en paramètre et rend la main (`delay(1)`) en attendant. C'est la règle
  d'or du kit (voir `PIEGES.md`).
- **La vérité est dans le matériel.** La vitesse se lit par
  `Serial.baudRate()`, jamais dans une copie.
- **Pas de NVS, pas de Wi-Fi, pas de HTTP** dans `src/` (hors pont WebSocket,
  qui reçoit hôte, port et chemin de l'appelant).
- **Fichiers `.cpp`** plutôt que des en-têtes à variables globales : les
  fichiers actuels ne tiennent que parce qu'ils sont inclus une seule fois.
- Dépendances : `Minitel1B_Hard` (obligatoire), `arduinoWebSockets`
  (seulement pour le pont). Pas d'ArduinoJson.

## Comment Minitel_Sigil la consommerait

Deux options, la première est recommandée.

1. **Sous-module dans le croquis**, épinglé sur un commit :
   `Minitel_Sigil/src/MinitelKit` → `#include "src/MinitelKit/src/MinitelKit.h"`.
   L'IDE Arduino compile récursivement le dossier `src/` d'un croquis : rien à
   installer, et le firmware reste reproductible (un flash = un commit du kit
   connu). Piège : aucun `.cpp` d'exemple ne doit traîner hors de `src/` du kit
   (les exemples sont en `.ino`, ignorés).
2. **Dossier `libraries/`** : cloner le kit dans `Documents/Arduino/libraries/`.
   Plus simple, mais la version utilisée n'est pas tracée par le firmware.
   Acceptable pour essayer, pas pour flasher le dongle en service.

Migration, un module à la fois, chacun dans son propre commit du firmware,
compilé et flashé avant le suivant : `MinitelText` → `MinitelKeys` →
`MinitelLink` (après stabilisation de la session série) → `G1Canvas` → yeux et
visages → `Mfa1` → pont WebSocket. Minitel_Face_Autonomous n'est pas migré
(reflashable tel quel, figé).

## Place dans l'unification (lemegeton-core/canon/UNIFICATION.md)

Le groupe « Visage Minitel » mêle aujourd'hui quatre dépôts de visibilités
différentes. Proposition :

| Couche | Dépôt | Visibilité |
| --- | --- | --- |
| Socle Minitel (C++, docs du terminal) | **minitel-kit** | public |
| Firmwares | Minitel-Sigil, Minitel-Face-Autonomous | privés, consomment le kit |
| Rendu serveur du visage | minitel-face | privé ; ses `g1.js`/`EscOptimizer` rejoignent `js/` du kit en phase 2 |
| Passerelle 3615 (serveur) | 3615-gateways | public, reste à part |

Ligne à ajouter au tableau de l'étape 4 de `UNIFICATION.md` (non modifié par
cette session) : *« minitel-kit (public) : socle commun du groupe Visage
Minitel ; les firmwares le consomment en sous-module. »*

Les règles G1 de `canon/VIDEOTEX.md` sont du savoir Vidéotex, pas du
personnage : elles ont leur place dans le kit (décision 5). Le canon garderait
un renvoi.

### 3615-gateways : rester à part

Recommandation : **ne pas fusionner**.

- C'est un **service** (Node : HTTP, WebSocket, Telnet, contenus Nitro,
  Lemegeton, Gwen Ha Star), pas une bibliothèque ; son cycle de vie est celui
  d'un serveur.
- Son exécution réelle vit dans Korigan (`services/3615-gateways`) ; le dépôt
  public est le prototype et la documentation du concept.
- Le kit côté appareil et la passerelle côté serveur se parlent par un
  **contrat** : octets Vidéotex bruts sur WebSocket (`?mode=raw`), touches
  remontées en binaire. Ce contrat se documente dans le kit
  (`VideotexWsBridge`) et la passerelle y renvoie.
- Seule `firmware/TELNET_PRO_PRESETS.md` de 3615-gateways a sa place côté kit
  (réglages du dongle) : un lien suffit.

## Ce qui ne doit jamais entrer dans le kit

Vérifié sur le squelette (voir « Audit » ci-dessous) :

- adresses du réseau local, noms d'hôtes et de machines de la maison ;
- mots de passe, y compris celui par défaut du compte admin des firmwares ;
- espaces et clés NVS des firmwares ;
- noms d'agents, routes et capacités de la mesh ;
- numéro de série du terminal ;
- chemins locaux de la machine, lettres de lecteurs, numéros de ports série.

Avant chaque publication : `tools/verifier-prive.sh` doit sortir vide. Il lit
ses motifs dans `.motifs-prives` (gitignoré, local) : la liste des valeurs
interdites ne doit pas elle-même être publiée.

## Audit du squelette (2026-10-16)

Le squelette a été passé à la recherche d'IP, de mots de passe, de clés et
espaces NVS, de noms de machines et de chemins locaux : aucune occurrence.
Les dépôts privés sont nommés (Minitel-Sigil, minitel-face) parce que le kit
doit dire d'où vient son code ; leur contenu n'est pas reproduit.

## Décisions à trancher

1. ~~**Nom**~~ : tranché le 2026-10-16, `minitel-kit` (bibliothèque
   `MinitelKit`).
2. ~~**Licence**~~ : tranché le 2026-10-16, **GPL-3.0** pour le code et les
   docs (`LICENSE`), alignée sur `Minitel1B_Hard`. La licence MIT de
   web-eye-animation, que transpose le moteur d'yeux, est reproduite dans
   `NOTICE` et doit suivre le code porté dans `src/faces/`.
3. **Fusion avec 3615-gateways** : recommandation « rester à part » (ci-dessus).
4. **Langue** : docs en français (comme les firmwares), README bilingue
   FR/EN comme minitel-musee ?
5. **Règles G1** : déplacer `canon/VIDEOTEX.md` dans le kit (le canon garde un
   renvoi), ou laisser le canon faire foi et le kit y renvoyer ?
6. **Visages et yeux** : dans `src/faces/` du kit (public, réutilisable), ou
   dans un dépôt Lemegeton à part ? Ils sont déjà décrits publiquement dans le
   canon ; le nom du visage « Zyra » est à confirmer pour un dépôt public.
7. **Phase 2 JavaScript** (`js/` avec `g1.js`, `EscOptimizer`, décodeur de
   minitel-musee) : oui / non / plus tard.
8. **Consommation** : sous-module épinglé (recommandé) ou dossier `libraries/`.
