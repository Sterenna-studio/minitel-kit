# Procédures de test sur le vrai terminal

Toutes passent par la **console série USB à 115 200 bauds** (commandes
terminées par Entrée). L'exemple `examples/ConsoleDeTest` les fournira ; elles
existent aujourd'hui dans le firmware Minitel_Sigil.

## VITESSE : ce que fait vraiment le port

```text
VITESSE
[Vitesse] ESP32 a 4800 bauds | le Minitel repond: 4800
```

Compare la vitesse **lue dans le matériel** (`Serial2.baudRate()`) et celle que
le terminal annonce (demande PRO1, bornée). Les deux doivent être égales. Un
`-1` côté Minitel : terminal éteint, figé, ou port à la mauvaise vitesse.

`VITESSE 4800` appelle `changeSpeed(4800)` et affiche son retour, la vitesse
du port après coup et la réponse du terminal : c'est le test qui a montré que
la bibliothèque change la vitesse du port dans le dos de l'appelant.

## DEBIT : le débit réel de la liaison

```text
DEBIT
[Debit] baud=4800 | 480 octets ecrits en 1000 ms
```

Écrit 480 octets en ligne 24 et mesure le temps. Attendu : ~1 s à 4800 bauds,
~4 s à 1200. Un écart d'un facteur quatre signe un port resté à 1200.

## TOUCHE <hex> : rejouer le clavier sans personne devant l'écran

```text
TOUCHE 1346
[Mesure] touche 0x1346 : 5400 ms, tampon libre 128 -> 0
```

Injecte un code de touche (même valeur que le lecteur clavier) dans le
traitement des touches, puis mesure la durée du traitement et le tampon
d'émission avant et après. Sert à rejouer un scénario de navigation et à
trouver l'écran qui bloque la boucle.

Codes usuels :

| Touche | Code |
| --- | --- |
| Envoi | `1341` |
| Retour | `1342` |
| Répétition | `1343` |
| Guide | `1344` |
| Annulation | `1345` |
| Sommaire | `1346` |
| Correction | `1347` |
| Suite | `1348` |
| Connexion/Fin | `1359` (non documenté) |

## Intégrité du tas

Après chaque touche, chaque changement d'écran et chaque seconde, appeler
`heap_caps_check_integrity_all(true)`. À la **première** corruption, écrire
dans le journal ce qui vient de se passer (touche, écran) puis se taire :
c'est ce qui désigne le code qui écrit hors de sa mémoire.

## Fil d'Ariane RTC

Trois mots en mémoire RTC non initialisée (`RTC_NOINIT_ATTR`) survivent à un
redémarrage logiciel ou au chien de garde : la dernière étape de `loop()`
atteinte, la dernière touche, le mode courant. Un mot « magique » dit si le
contenu est valide. Au démarrage :

```text
[Ariane] avant ce demarrage (raison 6) : etape 53, touche 0x1346, mode 0
```

La raison vient de `esp_reset_reason()`. Une étape fixe et une touche
répétée d'un redémarrage à l'autre désignent l'endroit exact du blocage.

## Mire : ce que le terminal affiche vraiment

Les étapes et leur cadence (3 s d'attente, puis environ 0,65 s par écran)
sont celles du laboratoire Vidéotex de la passerelle 3615 ; le contenu de
chaque écran est proposé ici. À regarder sur le vrai Minitel.

1. **Grille** : bords et coins aux quatre extrémités (lignes 1 et 24,
   colonnes 1 et 40).
2. **Couleurs** : les huit couleurs, texte et fond.
3. **Jeu de caractères** : G0 complet, y compris les codes qui diffèrent de
   l'ASCII, et les accents G2.
4. **ASCII** : un texte replié par `asciiFold`.
5. **Mosaïque** : les 64 combinaisons G1, puis une forme connue (yeux pleins).
6. **Attributs** : clignotement, inversion, soulignement, tailles.
7. **Bornes** : écrire en colonne 40 et en ligne 24 sans défilement.
8. **Image réelle**, puis quatre images d'une **animation**.

Avant de lancer la mire : renvoyer le terminal en Vidéotex 40 colonnes
(`ESC 3A 32 7E`), caractère normal (`ESC 4C`), curseur invisible (`0x14`).

## Campagne minimale avant un flash « en service »

1. Démarrage **Minitel éteint** : le firmware démarre, réessaie toutes les
   10 s, puis trouve le terminal à son allumage (1200 → 4800).
2. `VITESSE` : les deux valeurs égales à 4800.
3. `DEBIT` : ~1 s.
4. Parcours de tous les écrans au clavier réel, puis le même par `TOUCHE` :
   aucune touche au-delà du chien de garde, aucun `[Tas] CORROMPU`.
5. Éteindre le Minitel pendant un écran, le rallumer : la boucle survit.
6. Redémarrage forcé : le fil d'Ariane s'affiche et est cohérent.
