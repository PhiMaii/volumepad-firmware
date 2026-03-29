#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "config/AppConfig.h"
#include "config/RuntimeSettings.h"
#include "display/DisplayTypes.h"
#include "display/St7789Display.h"

namespace vp {

class DisplayManager {
 public:
  void begin(const DeviceConfig& config, const DeviceRuntimeSettings& settings);
  void onSettingsChanged(const DeviceRuntimeSettings& settings);

  void applyRenderPayload(JsonVariantConst payload);
  void tick(uint32_t nowMs);

 private:
  DeviceConfig config_{};
  DeviceRuntimeSettings settings_{};
  DisplayModel lastModel_{};
  St7789Display st7789_{};
  bool started_ = false;
};

}  // namespace vp