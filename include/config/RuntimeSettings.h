#pragma once

#include "config/AppConfig.h"
#include "util/MathUtil.h"

namespace vp {

struct DeviceRuntimeSettings {
  int detentCount = VP_DETENT_COUNT_DEFAULT;
  float detentStrength = VP_DETENT_STRENGTH_DEFAULT;
  float snapStrength = VP_SNAP_STRENGTH_DEFAULT;
  float ledBrightness = VP_LED_BRIGHTNESS_DEFAULT;
  float displayBrightness = VP_DISPLAY_BRIGHTNESS_DEFAULT;
  bool encoderInvert = false;
  int buttonLongPressMs = VP_BUTTON_LONG_PRESS_DEFAULT;

  void clamp() {
    detentCount = clampValue(detentCount, 1, 360);
    detentStrength = clampValue(detentStrength, 0.0f, 1.0f);
    snapStrength = clampValue(snapStrength, 0.0f, 1.0f);
    ledBrightness = clampValue(ledBrightness, 0.0f, 1.0f);
    displayBrightness = clampValue(displayBrightness, 0.0f, 1.0f);
    buttonLongPressMs = clampValue(buttonLongPressMs, 100, 5000);
  }
};

}  // namespace vp