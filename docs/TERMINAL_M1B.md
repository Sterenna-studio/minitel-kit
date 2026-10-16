# Fiche du terminal : Alcatel Minitel 1B (M1B)

Terminal de référence du kit : un **Minitel 1B** fabriqué par Telic Alcatel
(modèle M1B, type CU, 1986). C'est le modèle visé par la bibliothèque
[Minitel1B_Hard](https://github.com/eserandour/Minitel1B_Hard).

## Plaque signalétique

| Champ | Valeur |
| --- | --- |
| Fabricant | Telic Alcatel |
| Modèle | M1B (Minitel 1B), type CU |
| Mode 1 | Vidéotex CEPT, profil 225L-40C (40 colonnes) |
| Mode 2 | ASCII, profil 225L-80C (80 colonnes) |
| Alimentation | 220 V 50 Hz, 40 W |
| Date | 31.12.86 |

## Ce qui compte pour le code

- **Démarre en Vidéotex 40 colonnes**, à **1200 bauds**. Le firmware doit
  donc le détecter à 1200 puis demander 4800.
- **Vitesses** : 300, 1200 et **4800** bauds. **9600 n'existe que sur le
  Minitel 2** : ne pas compter dessus pour un 1B.
- **Liaison** : 7 bits, parité paire, 1 bit de stop (**7E1**). Masquer les
  octets reçus par `0x7F`.
- **Prise DIN périphérique** : TX de l'ESP32 vers l'entrée du Minitel, RX vers
  la sortie clavier, masse commune. Passer par une interface adaptée (le
  dongle iodeo Minitel-ESP32 l'intègre) : ne jamais injecter un niveau
  incompatible dans l'ESP32.
- **Écran** : 40 × 24 en Vidéotex ; la rangée 0 est la ligne de service.
  En mosaïque G1, une cellule = 2 × 3 pseudo-pixels : 80 × 72 au total.
- **Débit utile** : à 4800 bauds en 7E1 (10 bits par caractère), ~480
  caractères par seconde ; à 1200, ~120. Une page pleine de 800 caractères
  prend ~1,7 s à 4800 et ~6,7 s à 1200.
- **Trois standards** sur le 1B : Télétel Vidéotex (40 col., défaut), Télétel
  mixte (80 col.), Téléinformatique (80 ou 40 col.).

## Protocole utile

| Demande | Octets envoyés | Réponse |
| --- | --- | --- |
| Vitesse courante | `ESC 39 STATUS_VITESSE` (PRO1) | `1B 3A 75 xx` : `52` = 300, `64` = 1200, `76` = 4800, `7F` = 9600 |
| Écho clavier coupé | `ESC 3B AIGUILLAGE_OFF CODE_RECEPTION_MODEM CODE_EMISSION_CLAVIER` (PRO3) | acquittement `1B 3B 63 …` |
| Mode page (pas de défilement) | `ESC 3A STOP ROULEAU` (PRO2) | acquittement `1B 3A 73 …` |

| Passer en mode mixte (80 col.) | `ESC 3A 32 7D` (PRO2 MIXTE1) | `13 70` |
| Revenir en Vidéotex (40 col.) | `ESC 3A 32 7E` (PRO2 MIXTE2) | `13 71` |
| Passer en téléinformatique | `ESC 3A 31 7D` (PRO2 TELINFO) | `1B 5B 3F 7A` |
| Position du curseur | `ESC 61` | `US ligne colonne` (`0x40 +` chacune) |

Attention : `ESC 61` **demande** la position du curseur, il ne le cache pas.
Le terminal répond sur sa sortie, comme s'il s'agissait de touches.

Les constantes sont celles de `Minitel1B_Hard`. Référence : les STUM1B
(spécifications techniques d'utilisation du Minitel 1B), p. 118-124 pour le
clavier et p. 141 pour la vitesse.

## Affichage Vidéotex

| Élément | Codes |
| --- | --- |
| Curseur visible / invisible | `CON` `0x11` / `COFF` `0x14` |
| Effacer l'écran / curseur en haut | `FF` `0x0C` / `RS` `0x1E` |
| Positionner | `US` `0x1F`, `0x40 + ligne`, `0x40 + colonne` (ligne 0 = ligne de service) |
| Jeu G1 (mosaïque) / G0 (texte) | `SO` `0x0E` / `SI` `0x0F` |
| Couleur du caractère / du fond | `ESC 0x40 + n` / `ESC 0x50 + n` (0 noir … 7 blanc) |
| Taille normale / double hauteur / double grandeur | `ESC 4C` / `ESC 4D` / `ESC 4F` (texte seulement) |
| Effacer la fin de ligne | `CAN` `0x18` |

- **Couleurs** dans l'ordre des codes : noir, rouge, vert, jaune, bleu,
  magenta, cyan, blanc. Sur un tube noir et blanc elles deviennent des gris :
  noir 0 %, bleu 40 %, rouge 50 %, magenta 60 %, vert 70 %, cyan 80 %,
  jaune 90 %, blanc 100 %. Pour le contraste : blanc ou jaune sur noir.
- **Attribut « en espace »** : un changement de couleur est porté par la
  cellule qui suit le code. Terminer un segment par G0 + espace.
- **Double hauteur** : le caractère grandit **vers le haut** à partir de la
  ligne du curseur.
- **G0 n'est pas tout à fait l'ASCII** : `0x5E` est une flèche vers le haut,
  `0x60` un trait horizontal, `0x7B`–`0x7D` des traits verticaux, `0x7E` un trait haut. Les accents passent par
  le jeu G2 (`SS2` `0x19`, puis l'accent, puis la lettre) ; certains services
  d'époque utilisaient `SYN` (`0x16`) à la place de `SS2`.
- **Mode mixte** (80 colonnes) : plus de mosaïque G1, attributs réduits, et le
  curseur souligné clignotant ne se cache pas.

## Clavier

| Touche | Code renvoyé |
| --- | --- |
| Touches de fonction (Envoi, Retour, Suite, Sommaire, Guide, Répétition, Correction, Annulation, Connexion/Fin) | `0x13xx` (deux octets) |
| Lettres accentuées | `0x19 xx [yy]` (SS2, deux ou trois octets) |
| Flèches et clavier étendu | `0x1B 0x5B xx [yy]` |
| Esc seul | `0x1B` |

Sur un terminal Minitel, ce qui est tapé s'affiche puis attend **Envoi** ;
seules les touches de fonction agissent d'elles-mêmes.

## Remarques

- Le numéro de série du terminal n'a pas sa place ici.
- Un Minitel figé ne répond plus à la demande de vitesse : l'éteindre et le
  rallumer. Le firmware doit survivre à ce cas (voir [PIEGES.md](PIEGES.md)).
