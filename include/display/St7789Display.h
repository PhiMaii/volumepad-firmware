#pragma once

#include <Arduino.h>

#include "config/AppConfig.h"
#include "display/DisplayTypes.h"

namespace vp {

class St7789Display {
 public:
  static constexpr uint16_t kWidth = 76;
  static constexpr uint16_t kHeight = 284;

  void begin(const DeviceConfig& config);
  void setBrightness(float brightness);
  void render(const DisplayModel& model);

 private:
  DeviceConfig config_{};
  bool started_ = false;
};

}  // namespace vp