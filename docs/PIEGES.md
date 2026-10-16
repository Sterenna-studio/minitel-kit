# Pièges déjà payés

Chaque ligne ci-dessous a produit une vraie panne ou un défaut visible sur un
Minitel 1B piloté par un ESP32 (dongle iodeo Minitel-ESP32, core esp32 2.0.17,
bibliothèque `Minitel1B_Hard`). La colonne « Source » dit où le constat est
consigné à l'origine.

Sources :

- **[SIG]** README et code de Minitel_Sigil (firmware privé) ;
- **[FA]** README et `docs/` de Minitel_Face_Autonomous (firmware privé) ;
- **[MF]** `docs/` de minitel-face (serveur privé) ;
- **[CORE]** notes d'exploitation de la mesh qui pilote le Minitel (privé) ;
- **[CANON]** [`lemegeton-core/canon/VIDEOTEX.md`](https://github.com/Sterenna-studio/lemegeton-core/blob/main/canon/VIDEOTEX.md) (public) ;
- **[MUSEE]** [minitel-musee](https://github.com/Sterenna-studio/minitel-musee) : décodeur Vidéotex `src/videotex/decoder.ts` (public) ;
- **[GW]** [3615-gateways](https://github.com/Sterenna-studio/3615-gateways) (public) et sa version en service dans un dépôt privé ;
- **[LIB]** lecture directe du code de `Minitel1B_Hard` (`Minitel1B_Hard.cpp`).

La règle qui résume presque tout : **aucune attente de plus de ~100 ms sans
rendre la main, et aucune attente sans limite de temps.**

## 1. Bibliothèque Minitel1B_Hard

| Date | Piège | Conséquence | Parade | Source |
| --- | --- | --- | --- | --- |
| 2026-09-15 | `getKeyCode()` attend l'octet suivant d'une séquence (touches de fonction `0x13…`, accents `0x19…`, flèches `0x1B 0x5B…`) par `while (!available());`, sans limite. | Un seul octet perdu sur la liaison fige `loop()` pour toujours : écran, clavier, console et HTTP morts, sans redémarrage. | Assembler les séquences octet par octet à chaque tour de boucle, abandonner une séquence incomplète après 150 ms, `0x1B` seul = Esc après 20 ms. Mêmes codes que `getKeyCode(false)`. | [SIG] `MinitelKeys.h`, [CORE] |
| 2026-09-16 | `currentSpeed()` attend la réponse une seconde en boucle serrée, et `searchSpeed()` reboucle sans fin tant que le terminal ne répond pas. | Minitel éteint ou figé : huit essais d'affilée affament la tâche de repos, le chien de garde **matériel** réinitialise la puce (`rst:0x8 TG1WDT_SYS_RESET`) : boucle de redémarrage. | Refaire la demande de vitesse (PRO1 + `STATUS_VITESSE`) avec un délai et `delay(1)` en attendant ; une vitesse par essai, nombre d'essais borné par l'appelant ; démarrer sans terminal et réessayer toutes les 10 s. | [SIG] `.ino`, [CORE] |
| 2026-10-16 | `echo()` et `pageMode()` attendent leur acquittement **sans limite**. | Un octet perdu fige `loop()` jusqu'au chien de garde. | Envoyer les mêmes octets soi-même et attendre l'acquittement au plus 300 ms (`ESC 3B …` → `1B 3B 63`, `ESC 3A …` → `1B 3A 73`). | [SIG] `.ino` |
| 2026-10-16 | Le **constructeur** de `Minitel` ouvre `Serial2` à 1200 bauds avant `setup()`. | Deux effets. `setTxBufferSize()` appelé dans `setup()` échoue **en silence** (l'UART est déjà installé) : seule la FIFO matérielle de 128 octets a jamais servi. Et surtout, les constructeurs globaux tournent sur le **cœur 0** : l'interruption de la liaison s'y installait, à côté du Wi-Fi. Tous les plantages du 2026-10-16 (pile d'IDLE0, chien de garde des interruptions) étaient sur ce cœur. | Rouvrir le port **une seule fois** dans `setup()` (`end()`, 10 ms, `begin(1200, SERIAL_7E1, rx, tx)`), donc sur le cœur 1. Vérifier la valeur de retour de `setTxBufferSize()`. | [SIG] `.ino`, commit « liaison rouverte sur le cœur 1 » |
| — | Bilan des attentes de la bibliothèque, lu dans son code. **Sans limite** : `getKeyCode()`/`readKey()`, `echo()`, `aiguillage()`, `pageMode()`, `scrollMode()`, `smallMode()`, `capitalMode()`, `extendedKeyboard()`, `standardKeyboard()`, `connexion()`, `getCursorXY()`/`getCursorX()`/`getCursorY()`, `identificationBytes()`, `searchSpeed()`. **Bornées mais en boucle serrée** : `currentSpeed()`/`changeSpeed()` (1 s), `modeMixte()`/`modeVideotex()`/`standardTeleinformatique()` (100 ms). | Toute fonction de la première liste fige la boucle si un octet se perd. | Ne pas les appeler : envoyer les mêmes octets soi-même et attendre la réponse avec une limite (voir `MinitelLink`). | [LIB] |
| 2026-10-16 | `changeSpeed()` change la vitesse du port par `updateBaudRate()` dans le dos de l'appelant, **avant** de savoir si le terminal a accepté. | Une copie locale de la vitesse se désynchronise : l'ESP32 finit à 1200 bauds en se croyant à 4800, chaque écran quatre fois trop lent. | Ne jamais garder la vitesse dans une variable : la lire dans le matériel (`Serial2.baudRate()`), et revérifier par une demande au terminal après tout changement. | [SIG] `.ino` |

## 2. Liaison série et ESP32

| Date | Piège | Conséquence | Parade | Source |
| --- | --- | --- | --- | --- |
| — | La PSRAM du profil ESP32 reprend GPIO 16/17. | Plus de liaison Minitel. | Compiler avec **PSRAM désactivée**. | [FA], [SIG] |
| 2026-09-16 | Réinstaller le pilote UART à répétition (`end()`/`begin()`) pour changer de vitesse. | Chien de garde matériel (`rst:0x8`). | Changer de vitesse par `updateBaudRate()` seulement, puis ~20 ms de pause. | [SIG] `.ino` |
| 2026-10-16 | Tampon d'émission logiciel de 2 Ko (UART arrêté puis rouvert pour l'obtenir). | Un écran complet écrit octet par octet prenait jusqu'à 19 s contre 6 s sans tampon : plus lent que la ligne elle-même ; le chien de garde de la boucle (20 s) tombait. | Garder la FIFO matérielle pour l'instant. **Attention** : la mesure a été faite quand la liaison tournait en secret à 1200 bauds ; la conclusion n'est pas fiable et l'essai est à refaire à 4800. | [SIG] `.ino` |
| 2026-10-16 | Erreurs de ligne (break, parité, trame) en rafale : Minitel à une autre vitesse, câble qui bouge, allumage. | Le cœur est inondé d'interruptions pour rien. | Masquer ces interruptions (`uart_disable_intr_mask(UART_NUM_2, UART_BRK_DET_INT_ENA_M \| UART_PARITY_ERR_INT_ENA_M \| UART_FRM_ERR_INT_ENA_M)`) : un octet abîmé est de toute façon écarté par le lecteur clavier. | [SIG] `.ino` |
| 2026-10-16 | **Ouvert** : au démarrage, la première demande de vitesse peut provoquer un ou deux redémarrages (`TG1WDT`, sans message de panique) avant de réussir. | Démarrage plus long, sans perte. | Cause non trouvée ; le fil d'Ariane RTC est l'outil pour la chercher. | [SIG] commit « liaison rouverte sur le cœur 1 » |
| — | Écrire un écran de plus de ~500 octets d'un seul tenant depuis le rappel WebSocket. | Le Wi-Fi et le chien de garde sont affamés : redémarrage de l'ESP32. | Rendre la main (`yield()`) tous les 32 octets. | [MF] croquis `Minitel-Face-ESP32` |
| 2026-10-16 | Terminal muet laissé à 300 bauds entre deux essais. | Ce qui s'écrit en attendant bloque la boucle des dizaines de secondes (chien de garde). | Entre deux sondes, laisser le port à 9600 bauds : ce qui part vers un terminal absent part vite. | [SIG] `.ino` |
| — | Lire 8 bits sur une liaison 7E1. | Codes clavier faux. | Masquer `& 0x7F`. | [SIG] `MinitelKeys.h` |
| — | Une image pleine vers le Minitel ne s'envoie pas en un instant. | 800 caractères à 4800 bauds ≈ 1,7 s théoriques ; un flux d'images plus rapide s'accumule. | Plancher d'une image toutes les 2 s pour un flux ; n'envoyer que les cellules qui changent ; ne produire une image que si le tampon d'émission a de la place. | [CORE], [SIG] |
| — | Un regard qui saute d'un coup. | À 4800 bauds, un saut brusque se lit comme un défaut d'affichage. | Déplacements courts et lents. | [SIG] README |

## 3. Vidéotex et mosaïques G1

Les règles complètes et leurs vérifications sont dans [CANON] ; résumé des
défauts qui y ont mené :

| Date | Défaut | Cause | Source |
| --- | --- | --- | --- |
| 2026-10-06 | Barres **cisaillées** (bout droit arrondi à l'envers du gauche). | Deux octets différents en embouts d'une même barre : `0x70` (tiers bas) et `0x2F` (deux tiers hauts), sous des noms trompeurs `G1_TOP`/`G1_BOTTOM`. Chacun est symétrique sur lui-même : la barre penche. | [CANON], [FA] `docs/G1_MOSAIQUES.md` |
| 2026-10-16 | Yeux **en trois barres** au lieu de blocs pleins (LibEyes). | Trois constantes nommaient les lignes 9, 11 et 13 ; les lignes 10 et 12 n'étaient jamais peintes. | [CANON] |
| 2026-10-16 | Visages **troués, sans bords, décentrés**, bouches à l'envers, pupille invisible. | Nettoyage qui effaçait sans repeindre ; lignes sans bords ; sourire dessiné à l'envers ; pupille en couleur NOIR sur fond noir. | [CANON] |
| — | Une couleur « déborde » sur la cellule suivante. | L'attribut de couleur est « en espace » : il est absorbé par la cellule qui suit le code. | Terminer chaque segment par G0 + espace (`0x0F 0x20`) ; ce caractère tampon gomme ce qui est juste après, en tenir compte dans les nettoyages. [MF] `minitel_display_notes.md`, [CANON] point 9 |
| — | Clignotement à chaque image. | `CLS` avant chaque image. | Dessiner la structure une fois, puis ne patcher que les rangées qui changent. [MF] `display_strategy.md` |
| — | Un octet hors plage s'affiche comme une lettre. | Mosaïques valides : `0x20–0x3F` et `0x60–0x7F` seulement. | [CANON] |
| — | `ESC 0x61` utilisé pour « cacher le curseur » (`HIDE_CURSOR` de minitel-face, `1B 61` de ses notes). | C'est la **demande de position du curseur** (STUM p. 98) : le Minitel répond `US ligne colonne` sur sa sortie, des octets qui repartent vers l'ESP32 comme des touches. | Curseur invisible = `COFF` (`0x14`), visible = `CON` (`0x11`). Constat du 2026-10-16, pas encore corrigé dans minitel-face. [LIB], [MF] |
| — | Notes de minitel-face : double hauteur `1B 0D`, mode mixte `1B 3B 7A 50` / `1B 3B 7A 47`, téléinformatique « non activable à distance ». | Séquences fausses. | Double hauteur `ESC 0x4D` ; mixte `ESC 3A 32 7D` (acquit `13 70`), retour Vidéotex `ESC 3A 32 7E` (acquit `13 71`), téléinformatique `ESC 3A 31 7D`, activable à distance. [LIB], [MF] |
| — | Octets `0x40–0x5F` en mode G1. | Le canon les dit invalides ; le décodeur de minitel-musee, calé sur des pages d'époque (logo de Pinky), les dessine comme `0x60–0x7F`. | Pour un dessin à nous : rester dans `0x20–0x3F` / `0x60–0x7F`. Comportement du M1B réel à vérifier. [CANON], [MUSEE] |
| — | Texte UTF-8 brut à l'écran. | Les octets multi-octets s'affichent en caractères parasites. | Replier en ASCII avant d'écrire (`asciiFold`). [SIG] `Screen.h` |

À retenir : **une constante de ligne n'est pas une forme**, **embouts
identiques aux deux bouts**, **regarder la forme entière** avant de la
recopier ailleurs.

## 4. Mémoire de l'ESP32 (si le Minitel partage la carte avec du Bluetooth)

Rien de Minitel, mais payé sur la même carte :

| Date | Piège | Conséquence | Parade | Source |
| --- | --- | --- | --- | --- |
| 2026-10-16 | Libérer la réserve BLE dès le démarrage pour gagner de la RAM. | Quelques minutes plus tard, le cœur 0 (Wi-Fi) part en panique (« stack canary ») et le gestionnaire de panique boucle sans redémarrer : écran figé. | Ne rendre la réserve BLE qu'au démarrage d'A2DP. | [SIG] README |
| 2026-10-16 | Relancer le Wi-Fi après A2DP sans redémarrer. | `abort()`. | Redémarrer l'ESP32 en profil réseau. | [SIG] README |
| — | Wi-Fi et A2DP ensemble. | Ne tiennent pas en RAM ; la pile Bluetooth coûte ~85 Ko de tas libre même éteinte. | Profils exclusifs ; A2DP exige ≥ 96 Kio libres et un bloc contigu de 64 Kio. | [SIG], [FA] |

## 5. Réseau autour du Minitel

| Piège | Parade | Source |
| --- | --- | --- |
| `arduinoWebSockets` envoie par défaut `Origin: file://`, refusé (403) par un portail qui filtre l'origine. | N'envoyer aucun en-tête `Origin`. | [SIG] README |
| Un nom `.local` laissé à la bibliothèque WebSocket est résolu par mDNS à **chaque** reconnexion et gèle la boucle. | Le résoudre une fois à l'ouverture, se connecter à l'adresse. | [SIG] README |
| La résolution mDNS depuis une tâche planifiée côté serveur n'est pas fiable. | Adresse littérale et bail DHCP réservé. | [CORE] |
| Une requête réseau dans `loop()` fige l'écran et le clavier. | Toutes les requêtes dans une tâche séparée. | [SIG] README |
| Un client Telnet (pont série, firmware Telnet Pro, caméra) envoie des négociations `IAC` (`0xFF …`). | Le serveur doit les filtrer avant d'interpréter les touches ; ne jamais les renvoyer au Minitel. | [GW] |
| Un serveur WebSocket brut ne sait pas qui vient de se connecter. | Contrat de minitel-face : l'ESP32 envoie d'abord un message binaire (un octet `0x00`), le serveur répond par l'écran d'accueil. | [MF] croquis `Minitel-Face-ESP32` |
| Firmware `Minitel1B_Telnet_Pro` d'iodeo : réglages par défaut. | Écho **désactivé**, défilement activé, 80 colonnes et Prestel désactivés pour les premiers essais ; ping WebSocket 30 s ; essayer le réseau local avant toute adresse publique. | [GW] `firmware/TELNET_PRO_PRESETS.md` |

## 6. Flasher l'ESP32

| Piège | Parade | Source |
| --- | --- | --- |
| Après une mise à jour par le Wi-Fi (OTA), l'ESP32 peut démarrer sur `app1`. Réécrire seulement `app0` (0x10000) par USB n'a alors **aucun effet**. | Écrire aussi `boot_app0.bin` à 0xe000, comme le fait `arduino-cli upload`. | [SIG] README |
| Un envoi OTA coupé à la moitié. | Sans danger si l'image est validée en entier avant de basculer : la version en service reste intacte. | [SIG] README |

## 7. Diagnostiquer avant de soupçonner le réseau

Si l'ESP32 devient muet : **lire la console série à 115 200 bauds**. Si le
démarrage s'arrête sur la détection de vitesse, c'est le Minitel qui est figé :
l'éteindre et le rallumer. Voir [TESTS.md](TESTS.md) pour les outils (fil
d'Ariane, intégrité du tas, rejeu clavier).
