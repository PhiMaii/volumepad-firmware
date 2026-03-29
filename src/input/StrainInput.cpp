#include "input/StrainInput.h"

#include <cmath>

namespace vp {

bool StrainInput::begin(const DeviceConfig& config) {
  config_ = config;

  if (config_.pins.hx711Dout < 0 || config_.pins.hx711Sck < 0) {
    return false;
  }

  hx711_.begin(config_.pins.hx711Dout, config_.pins.hx711Sck);
  ready_ = true;
  return true;
}

StrainEvent StrainInput::poll(uint32_t nowMs) {
  StrainEvent event;
  if (!ready_ || !hx711_.is_ready()) {
    return event;
  }

  const long raw = hx711_.read();
  const float rawf = static_cast<float>(raw);

  if (!initialized_) {
    baseline_ = rawf;
    filtered_ = rawf;
    initialized_ = true;
    return event;
  }

  constexpr float filterAlpha = 0.25f;
  filtered_ += (rawf - filtered_) * filterAlpha;

  if (!pressed_) {
    baseline_ += (filtered_ - baseline_) * config_.hardware.strainBaselineAlpha;
  }

  float force = (baseline_ - filtered_) * config_.hardware.strainForceScale;
  if (!config_.hardware.strainInvertSign) {
    force = -force;
  }
  event.force = force;

  const float pressThreshold = config_.hardware.strainPressThreshold;
  const float releaseThreshold = pressThreshold - config_.hardware.strainReleaseHysteresis;
  const uint32_t debounceMs = 30;

  if (!pressed_ && force >= pressThreshold && (nowMs - lastEdgeMs_) >= debounceMs) {
    pressed_ = true;
    lastEdgeMs_ = nowMs;
    event.changed = true;
    event.pressed = true;
    event.triggerClick = true;
    return event;
  }

  if (pressed_ && force <= releaseThreshold && (nowMs - lastEdgeMs_) >= debounceMs) {
    pressed_ = false;
    lastEdgeMs_ = nowMs;
    event.changed = true;
    event.pressed = false;
    event.triggerClick = false;
  }

  return event;
}

bool StrainInput::ready() const {
  return ready_;
}

}  // namespace vp