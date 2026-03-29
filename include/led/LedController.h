#pragma once

#include <Arduino.h>

#include "config/AppConfig.h"
#include "config/RuntimeSettings.h"

namespace vp {

struct MeterModel {
  float rms = 0.0f;
  float peak = 0.0f;
  bool muted = false;
  float brightness = VP_LED_BRIGHTNESS_DEFAULT;
  String theme = "audio-default";
};

class LedController {
 public:
  void begin(const DeviceConfig& config, const DeviceRuntimeSettings& settings);
  void onSettingsChanged(const DeviceRuntimeSettings& settings);

  void setKeyPressed(uint8_t keyIndex, bool pressed);
  void applyMeter(const MeterModel& meter);
  void tick(uint32_t nowMs);

 private:
  DeviceConfig config_{};
  DeviceRuntimeSettings settings_{};
  MeterModel meter_{};

  bool keyPressed_[3] = {false, false, false};
  bool started_ = false;
  uint32_t lastShowMs_ = 0;
};

}  // namespace vp