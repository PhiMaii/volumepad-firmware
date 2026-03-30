#include "input/StrainInput.h"

#include <cmath>

#include "util/MathUtil.h"

namespace vp {

bool StrainInput::begin(const DeviceConfig& config) {
  config_ = config;

  pressThreshold_ = config_.hardware.strainPressThreshold;
  releaseHysteresis_ = config_.hardware.strainReleaseHysteresis;
  forceScale_ = config_.hardware.strainForceScale;
  baselineAlpha_ = config_.hardware.strainBaselineAlpha;

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
    baseline_ += (filtered_ - baseline_) * baselineAlpha_;
  }

  float force = (baseline_ - filtered_) * forceScale_;
  if (!config_.hardware.strainInvertSign) {
    force = -force;
  }

  lastForce_ = force;
  event.force = force;

  const float releaseThreshold = pressThreshold_ - releaseHysteresis_;
  const uint32_t debounceMs = 30;

  if (!pressed_ && force >= pressThreshold_ && (nowMs - lastEdgeMs_) >= debounceMs) {
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

void StrainInput::calibrateBaseline() {
  baseline_ = filtered_;
}

void StrainInput::applyDebugTuning(float pressThreshold, float releaseHysteresis, float forceScale, float baselineAlpha) {
  pressThreshold_ = pressThreshold;
  releaseHysteresis_ = clampValue(releaseHysteresis, 0.0f, pressThreshold_);
  forceScale_ = forceScale;
  baselineAlpha_ = clampValue(baselineAlpha, 0.00001f, 0.2f);
}

StrainDebugState StrainInput::getDebugState() const {
  StrainDebugState state;
  state.ready = ready_;
  state.pressed = pressed_;
  state.force = lastForce_;
  state.baseline = baseline_;
  state.filtered = filtered_;
  state.pressThreshold = pressThreshold_;
  state.releaseHysteresis = releaseHysteresis_;
  state.forceScale = forceScale_;
  state.baselineAlpha = baselineAlpha_;
  return state;
}

}  // namespace vp