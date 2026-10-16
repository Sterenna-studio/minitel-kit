#pragma once
#include <Arduino.h>
#include <Minitel1B_Hard.h>

// Lien avec le terminal : vitesse, commandes protocole, sonde.
// A porter depuis Minitel_Sigil (interrogerVitesseMinitel, commandeMinitel,
// vitesseMinitelPort, reglerVitesseMinitelPort, probeMinitelTerminal) une
// fois leur version stabilisee.
//
// - Toutes les attentes sont bornees et rendent la main (delay(1)).
// - La vitesse se lit TOUJOURS dans le materiel (port.baudRate()) :
//   Minitel::changeSpeed() la change par updateBaudRate dans le dos de
//   l'appelant, avant meme de savoir si le terminal a accepte.
// - Jamais Minitel::searchSpeed(), currentSpeed(), echo(), pageMode() : leurs
//   attentes sont sans limite ou en boucle serree.

namespace minitelkit {

class MinitelLink {
 public:
  // `port` est celui passe au constructeur de `minitel` (deja ouvert a
  // 1200 bauds par ce constructeur, avant setup()).
  MinitelLink(Minitel& minitel, HardwareSerial& port);

  // Demande sa vitesse au terminal (PRO1 + STATUS_VITESSE). -1 sans reponse.
  int askSpeed(uint32_t timeoutMs = 600);

  // Envoie `n` octets et attend une trame dont les 3 premiers octets valent
  // `ack`, au plus `timeoutMs`.
  bool command(const uint8_t* bytes, uint8_t n, uint32_t ack, uint32_t timeoutMs = 300);

  // Vitesse reelle du port.
  int portSpeed() const;
  // Change la vitesse du port par updateBaudRate seulement (jamais end/begin).
  void setPortSpeed(int baud);

  // Un essai par vitesse candidate, `attempts` au plus ; passe a `targetBaud`
  // si possible, coupe l'echo et met le mode page. Sans reponse, laisse le
  // port a `idleBaud` (rapide) jusqu'au prochain essai.
  bool probe(uint8_t attempts, int targetBaud = 4800, int idleBaud = 9600);

  bool ready() const { return ready_; }

 private:
  Minitel& minitel_;
  HardwareSerial& port_;
  uint8_t speedIndex_ = 0;
  bool ready_ = false;
};

}  // namespace minitelkit
