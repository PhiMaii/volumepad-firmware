#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "config/AppConfig.h"
#include "config/SettingsModel.h"

namespace vp {

struct HapticsDebugState {
  bool ready = false;
  int position = 0;
  float shaftAngle = 0.0f;
  float appliedVoltage = 0.0f;
  float endstopMinRad = -kEndstopRangeRad;
  float endstopMaxRad = kEndstopRangeRad;
};

class HapticsController {
 public:
  bool begin(const DeviceConfig& config, const NormalSettings& settings, const DebugTuning& tuning);
  void applySettings(const NormalSettings& settings, const DebugTuning& tuning);
  void tick(uint32_t nowMs);

  int consumeDeltaSteps();
  int position() const;
  bool ready() const;
  bool diagActive() const;
  HapticsDebugState getDebugState() const;

 private:
  bool readRawAngle(uint16_t& raw) const;
  void disableMotorPins() const;
  float rawToAngle(uint16_t raw) const;
  int encoderStepsPerRevolution() const;

  DeviceConfig config_{};
  NormalSettings settings_{};
  DebugTuning tuning_{};

  bool ready_ = false;
  int currentPosition_ = 0;
  int pendingDelta_ = 0;
  int32_t stepAccumulator_ = 0;
  uint16_t lastRawAngle_ = 0;
  bool hasLastRaw_ = false;
  float lastAngle_ = 0.0f;
  float lastAppliedVoltage_ = 0.0f;
};

}  // namespace vp
