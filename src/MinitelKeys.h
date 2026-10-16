#pragma once
#include <Arduino.h>

// Lecture non bloquante du clavier. A porter depuis MinitelKeys.h de
// Minitel_Sigil (identique a celui de Minitel_Face_Autonomous).
//
// Assemble les sequences (0x13.., 0x19.., 0x1B 0x5B..) octet par octet et
// renvoie EXACTEMENT les codes de Minitel::getKeyCode(false) : les constantes
// ENVOI, SOMMAIRE, TOUCHE_FLECHE_*... restent valables. Une sequence
// incomplete est abandonnee apres `sequenceTimeoutMs` au lieu de bloquer.

namespace minitelkit {

class MinitelKeys {
 public:
  explicit MinitelKeys(Stream& port, uint32_t escWaitMs = 20, uint32_t sequenceTimeoutMs = 150);

  // A appeler a chaque tour de loop(). 0 tant qu'aucune touche complete.
  uint32_t poll();
  void reset();
  uint32_t droppedSequences() const { return dropped_; }

  // Journal optionnel (sequence abandonnee...). nullptr = silencieux.
  void onLog(void (*log)(const char* message)) { log_ = log; }

 private:
  Stream& port_;
  uint32_t escWaitMs_, sequenceTimeoutMs_;
  uint8_t bytes_[4] = {0, 0, 0, 0};
  uint8_t length_ = 0;
  uint32_t startedAt_ = 0;
  uint32_t dropped_ = 0;
  void (*log_)(const char*) = nullptr;
};

}  // namespace minitelkit
