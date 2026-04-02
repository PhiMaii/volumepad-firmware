#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "config/SettingsModel.h"

namespace vp {

class SettingsStore {
 public:
  bool begin();
  void load(NormalSettings& settings, DebugTuning& tuning);
  bool saveSettings(const NormalSettings& settings);
  bool saveDebugTuning(const DebugTuning& tuning);

 private:
  static constexpr uint8_t kSchemaVersion = 1;
  Preferences prefs_;
  bool started_ = false;
};

}  // namespace vp
