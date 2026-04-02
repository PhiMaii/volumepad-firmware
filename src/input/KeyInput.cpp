#include "input/KeyInput.h"

namespace vp {

void KeyInput::begin(const DeviceConfig& config) {
  pins_[0] = config.pins.button1;
  pins_[1] = config.pins.button2;
  pins_[2] = config.pins.button3;

  for (uint8_t i = 0; i < kKeyCount; ++i) {
    if (pins_[i] < 0) {
      continue;
    }
    pinMode(pins_[i], INPUT_PULLUP);
    const bool pressed = (digitalRead(pins_[i]) == LOW);
    rawPressed_[i] = pressed;
    stablePressed_[i] = pressed;
    changedAtMs_[i] = millis();
  }
}

size_t KeyInput::poll(KeyPressEvent* outEvents, size_t maxEvents, uint32_t nowMs) {
  size_t emitted = 0;

  for (uint8_t i = 0; i < kKeyCount; ++i) {
    if (pins_[i] < 0) {
      continue;
    }

    const bool raw = (digitalRead(pins_[i]) == LOW);
    if (raw != rawPressed_[i]) {
      rawPressed_[i] = raw;
      changedAtMs_[i] = nowMs;
    }

    if ((nowMs - changedAtMs_[i]) < kDebounceMs) {
      continue;
    }

    if (rawPressed_[i] == stablePressed_[i]) {
      continue;
    }

    stablePressed_[i] = rawPressed_[i];
    if (!stablePressed_[i]) {
      continue;
    }

    if (outEvents != nullptr && emitted < maxEvents) {
      KeyPressEvent event;
      event.buttonId = static_cast<uint8_t>(i + 1U);
      outEvents[emitted++] = event;
    }
  }

  return emitted;
}

}  // namespace vp
