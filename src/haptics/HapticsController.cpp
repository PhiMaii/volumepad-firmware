#include "haptics/HapticsController.h"

#include <cmath>

#include "util/MathUtil.h"

namespace vp {

namespace {

constexpr uint8_t kAs5600Address = 0x36;
constexpr uint8_t kAs5600AngleRegHigh = 0x0C;
constexpr uint16_t kAs5600CountsPerTurn = 4096;
constexpr int kWrapHalfCounts = kAs5600CountsPerTurn / 2;
constexpr int kMinEncoderStepsPerTurn = 24;
constexpr int kMaxEncoderStepsPerTurn = 1024;
constexpr int kEncoderResolutionMultiplier = 8;

}  // namespace

bool HapticsController::begin(const DeviceConfig& config, const NormalSettings& settings, const DebugTuning& tuning) {
  ready_ = false;
  config_ = config;
  settings_ = settings;
  tuning_ = tuning;
  settings_.normalize();
  tuning_.normalize();

  pinMode(config_.pins.diag, INPUT_PULLUP);
  disableMotorPins();
  Wire.begin(config_.pins.i2cSda, config_.pins.i2cScl);

  uint16_t raw = 0;
  bool sampled = false;
  for (int i = 0; i < 8; ++i) {
    if (readRawAngle(raw)) {
      sampled = true;
      break;
    }
    delay(2);
  }

  if (!sampled) {
    return false;
  }

  const float angle = rawToAngle(raw);
  lastRawAngle_ = raw;
  hasLastRaw_ = true;
  lastAngle_ = angle;
  currentPosition_ = 0;
  stepAccumulator_ = 0;

  lastAppliedVoltage_ = 0.0f;
  ready_ = true;
  return true;
}

void HapticsController::applySettings(const NormalSettings& settings, const DebugTuning& tuning) {
  settings_ = settings;
  tuning_ = tuning;
  settings_.normalize();
  tuning_.normalize();
}

void HapticsController::tick(uint32_t nowMs) {
  (void)nowMs;
  if (!ready_) {
    return;
  }

  uint16_t raw = 0;
  if (!readRawAngle(raw)) {
    return;
  }

  if (!hasLastRaw_) {
    lastRawAngle_ = raw;
    hasLastRaw_ = true;
    return;
  }

  int deltaRaw = static_cast<int>(raw) - static_cast<int>(lastRawAngle_);
  if (deltaRaw > kWrapHalfCounts) {
    deltaRaw -= kAs5600CountsPerTurn;
  } else if (deltaRaw < -kWrapHalfCounts) {
    deltaRaw += kAs5600CountsPerTurn;
  }

  lastRawAngle_ = raw;
  lastAngle_ += (static_cast<float>(deltaRaw) * kTwoPi) / static_cast<float>(kAs5600CountsPerTurn);

  const int direction = settings_.encoderInvert ? -1 : 1;
  const int32_t scaled =
      static_cast<int32_t>(deltaRaw) * static_cast<int32_t>(encoderStepsPerRevolution()) * direction;
  stepAccumulator_ += scaled;

  int emitted = 0;
  while (stepAccumulator_ >= static_cast<int32_t>(kAs5600CountsPerTurn)) {
    stepAccumulator_ -= static_cast<int32_t>(kAs5600CountsPerTurn);
    ++emitted;
  }
  while (stepAccumulator_ <= -static_cast<int32_t>(kAs5600CountsPerTurn)) {
    stepAccumulator_ += static_cast<int32_t>(kAs5600CountsPerTurn);
    --emitted;
  }

  if (emitted != 0) {
    pendingDelta_ += emitted;
    currentPosition_ += emitted;
  }

  // Haptics are intentionally disabled in this mode.
  lastAppliedVoltage_ = 0.0f;
}

int HapticsController::consumeDeltaSteps() {
  const int delta = pendingDelta_;
  pendingDelta_ = 0;
  return delta;
}

int HapticsController::position() const {
  return currentPosition_;
}

bool HapticsController::ready() const {
  return ready_;
}

bool HapticsController::diagActive() const {
  if (config_.pins.diag < 0) {
    return false;
  }
  return digitalRead(config_.pins.diag) == LOW;
}

HapticsDebugState HapticsController::getDebugState() const {
  HapticsDebugState state;
  state.ready = ready_;
  state.position = currentPosition_;
  state.shaftAngle = lastAngle_;
  state.appliedVoltage = lastAppliedVoltage_;
  state.endstopMinRad = tuning_.endstopMinPos * kEndstopRangeRad;
  state.endstopMaxRad = tuning_.endstopMaxPos * kEndstopRangeRad;
  return state;
}

bool HapticsController::readRawAngle(uint16_t& raw) const {
  Wire.beginTransmission(kAs5600Address);
  Wire.write(kAs5600AngleRegHigh);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(static_cast<int>(kAs5600Address), 2) != 2) {
    return false;
  }

  const uint16_t high = static_cast<uint16_t>(Wire.read());
  const uint16_t low = static_cast<uint16_t>(Wire.read());
  raw = static_cast<uint16_t>(((high << 8U) | low) & 0x0FFFU);
  return true;
}

void HapticsController::disableMotorPins() const {
  const int pins[] = {
      config_.pins.ul,
      config_.pins.uh,
      config_.pins.vl,
      config_.pins.vh,
      config_.pins.wl,
      config_.pins.wh,
  };
  for (int pin : pins) {
    if (pin < 0) {
      continue;
    }
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
}

float HapticsController::rawToAngle(uint16_t raw) const {
  return (static_cast<float>(raw) * kTwoPi) / static_cast<float>(kAs5600CountsPerTurn);
}

int HapticsController::encoderStepsPerRevolution() const {
  const int base = (settings_.detentCount > 0) ? settings_.detentCount : 24;
  const int scaled = base * kEncoderResolutionMultiplier;
  return clampValue(scaled, kMinEncoderStepsPerTurn, kMaxEncoderStepsPerTurn);
}

}  // namespace vp
