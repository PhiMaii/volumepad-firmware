#pragma once

#include <Arduino.h>

#include "config/AppConfig.h"

namespace vp {

struct KeyEvent {
  uint8_t keyIndex;
  bool pressed;
};

class KeyInput {
 public:
  void begin(const DeviceConfig& config);
  size_t poll(KeyEvent* outEvents, size_t maxEvents, uint32_t nowMs);

 private:
  static constexpr uint8_t kKeyCount = 3;

  int pins_[kKeyCount] = {-1, -1, -1};
  bool stablePressed_[kKeyCount] = {false, false, false};
  bool rawPressed_[kKeyCount] = {false, false, false};
  uint32_t changedAtMs_[kKeyCount] = {0, 0, 0};
};

}  // namespace vp