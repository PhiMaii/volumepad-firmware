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

class StrainInput {
 public:
  bool begin(const DeviceConfig& config);
  StrainEvent poll(uint32_t nowMs);
  bool ready() const;

 private:
  DeviceConfig config_{};
  HX711 hx711_;

  bool ready_ = false;
  bool pressed_ = false;
  uint32_t lastEdgeMs_ = 0;

  float baseline_ = 0.0f;
  float filtered_ = 0.0f;
  bool initialized_ = false;
};

}  // namespace vp