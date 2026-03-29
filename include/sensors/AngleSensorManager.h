#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SimpleFOC.h>

#include "config/AppConfig.h"

namespace vp {

class AngleSensorManager {
 public:
  AngleSensorManager();

  bool begin(const DeviceConfig& config);
  Sensor* sensor();
  bool ready() const;
  float lastAngle() const;

 private:
  static AngleSensorManager* instance_;

  static float readAngleCallback();
  static void initCallback();

  void initBus();
  float readAngleRad();
  bool readAs5600Raw(uint16_t& raw) const;
  bool readMt6701Raw(uint16_t& raw) const;

  GenericSensor sensor_;
  DeviceConfig config_{};
  bool ready_ = false;
  mutable float cachedAngle_ = 0.0f;
};

}  // namespace vp