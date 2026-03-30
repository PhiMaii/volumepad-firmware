#pragma once

#include <Arduino.h>
#include <SimpleFOC.h>

#include <memory>

#include "config/AppConfig.h"
#include "config/RuntimeSettings.h"

namespace vp {

struct HapticsDebugState {
  bool ready = false;
  int position = 0;
  float detentStrengthMaxVPerRad = 0.0f;
  float snapStrengthMaxVPerRad = 0.0f;
  float clickPulseVoltage = 0.0f;
  uint32_t clickPulseMs = 0;
  float endstopMinPos = 0.0f;
  float endstopMaxPos = 0.0f;
  float endstopMinStrength = 0.0f;
  float endstopMaxStrength = 0.0f;
};

class HapticsController {
 public:
  bool begin(const DeviceConfig& config, Sensor* sensor);
  void applySettings(const DeviceRuntimeSettings& settings);

  void tick(uint32_t nowMs);
  void triggerClickPulse(uint32_t nowMs);

  int consumeDetentDelta();
  int currentPosition() const;
  bool ready() const;

  void applyDebugTuning(float detentStrengthMaxVPerRad,
                        float snapStrengthMaxVPerRad,
                        float clickPulseVoltage,
                        uint32_t clickPulseMs,
                        float endstopMinPos,
                        float endstopMaxPos,
                        float endstopMinStrength,
                        float endstopMaxStrength);
  HapticsDebugState getDebugState() const;

 private:
  float computeDetentTorque(float angle) const;
  float computeSnapTorque(float detentError) const;
  float computeEndstopTorque(float angle) const;
  float computeClickTorque(uint32_t nowMs);

  DeviceConfig config_{};
  DeviceRuntimeSettings settings_{};

  std::unique_ptr<BLDCMotor> motor_;
  std::unique_ptr<BLDCDriver6PWM> driver_;

  bool ready_ = false;
  int currentDetentIndex_ = 0;
  int lastDetentIndex_ = 0;
  int pendingDelta_ = 0;
  float detentOffset_ = 0.0f;

  bool clickActive_ = false;
  uint32_t clickStartMs_ = 0;
};

}  // namespace vp