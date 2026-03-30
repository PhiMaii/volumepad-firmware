#pragma once

#include <Arduino.h>
#include <HX711.h>

#include "config/AppConfig.h"

namespace vp {

struct StrainEvent {
  bool changed = false;
  bool pressed = false;
  bool triggerClick = false;
  float force = 0.0f;
};

struct StrainDebugState {
  bool ready = false;
  bool pressed = false;
  float force = 0.0f;
  float baseline = 0.0f;
  float filtered = 0.0f;
  float pressThreshold = 0.0f;
  float releaseHysteresis = 0.0f;
  float forceScale = 1.0f;
  float baselineAlpha = 0.0025f;
};

class StrainInput {
 public:
  bool begin(const DeviceConfig& config);
  StrainEvent poll(uint32_t nowMs);
  bool ready() const;

  void calibrateBaseline();
  void applyDebugTuning(float pressThreshold, float releaseHysteresis, float forceScale, float baselineAlpha);
  StrainDebugState getDebugState() const;

 private:
  DeviceConfig config_{};
  HX711 hx711_;

  bool ready_ = false;
  bool pressed_ = false;
  uint32_t lastEdgeMs_ = 0;

  float baseline_ = 0.0f;
  float filtered_ = 0.0f;
  float lastForce_ = 0.0f;
  bool initialized_ = false;

  float pressThreshold_ = 0.0f;
  float releaseHysteresis_ = 0.0f;
  float forceScale_ = 1.0f;
  float baselineAlpha_ = 0.0025f;
};

}  // namespace vp