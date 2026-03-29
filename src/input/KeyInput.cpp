#include "input/KeyInput.h"

namespace vp {

void KeyInput::begin(const DeviceConfig& config) {
  pins_[0] = config.pins.key1;
  pins_[1] = config.pins.key2;
  pins_[2] = config.pins.key3;

  for (uint8_t i = 0; i < kKeyCount; ++i) {
    if (pins_[i] >= 0) {
      pinMode(pins_[i], INPUT_PULLUP);
      const bool pressed = digitalRead(pins_[i]) == LOW;
      stablePressed_[i] = pressed;
      rawPressed_[i] = pressed;
      changedAtMs_[i] = millis();
    }
  }
}

size_t KeyInput::poll(KeyEvent* outEvents, size_t maxEvents, uint32_t nowMs) {
  constexpr uint32_t debounceMs = 18;
  size_t emitted = 0;

  for (uint8_t i = 0; i < kKeyCount; ++i) {
    if (pins_[i] < 0) {
      continue;
    }

    const bool raw = digitalRead(pins_[i]) == LOW;
    if (raw != rawPressed_[i]) {
      rawPressed_[i] = raw;
      changedAtMs_[i] = nowMs;
    }

    if ((nowMs - changedAtMs_[i]) < debounceMs) {
      continue;
    }

    if (stablePressed_[i] == rawPressed_[i]) {
      continue;
    }

    stablePressed_[i] = rawPressed_[i];
    if (outEvents != nullptr && emitted < maxEvents) {
      outEvents[emitted++] = KeyEvent{i, stablePressed_[i]};
    }
  }

  return emitted;
}

}  // namespace vp