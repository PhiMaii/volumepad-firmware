#pragma once

#include <Arduino.h>

#include "util/ColorUtil.h"
#include "util/MathUtil.h"

namespace vp {

enum class MeterMode {
  RingFill,
  VuPeakHold,
  PeakIndicator,
};

inline MeterMode parseMeterMode(const String& mode) {
  if (mode == "vu_peak_hold") {
    return MeterMode::VuPeakHold;
  }
  if (mode == "peak_indicator") {
    return MeterMode::PeakIndicator;
  }
  return MeterMode::RingFill;
}

inline const char* meterModeToString(MeterMode mode) {
  switch (mode) {
    case MeterMode::VuPeakHold:
      return "vu_peak_hold";
    case MeterMode::PeakIndicator:
      return "peak_indicator";
    case MeterMode::RingFill:
    default:
      return "ring_fill";
  }
}

struct NormalSettings {
  bool autoReconnectOnError = true;
  bool autoConnectOnStartup = false;
  float volumeStepSize = 0.02f;
  int detentCount = 24;
  float detentStrength = 0.65f;
  float snapStrength = 0.40f;
  bool encoderInvert = false;
  float ledBrightness = 0.80f;
  MeterMode meterMode = MeterMode::RingFill;
  String meterColor = "#00D26A";
  float meterBrightness = 0.80f;
  float meterSmoothing = 0.25f;
  uint32_t meterPeakHoldMs = 500;
  uint32_t meterMuteRedDurationMs = 700;
  bool lowEndstopEnabled = true;
  float lowEndstopPosition = -1.0f;
  float lowEndstopStrength = 0.70f;
  bool highEndstopEnabled = true;
  float highEndstopPosition = 1.0f;
  float highEndstopStrength = 0.70f;

  void normalize() {
    volumeStepSize = clampValue(volumeStepSize, 0.001f, 0.20f);
    detentCount = clampValue(detentCount, 0, 128);
    detentStrength = clampValue(detentStrength, 0.0f, 1.0f);
    snapStrength = clampValue(snapStrength, 0.0f, 1.0f);
    ledBrightness = clampValue(ledBrightness, 0.0f, 1.0f);
    meterBrightness = clampValue(meterBrightness, 0.0f, 1.0f);
    meterSmoothing = clampValue(meterSmoothing, 0.0f, 1.0f);
    meterPeakHoldMs = clampValue(meterPeakHoldMs, 0U, 3000U);
    meterMuteRedDurationMs = clampValue(meterMuteRedDurationMs, 50U, 3000U);
    lowEndstopPosition = clampValue(lowEndstopPosition, -1.0f, 1.0f);
    highEndstopPosition = clampValue(highEndstopPosition, -1.0f, 1.0f);
    lowEndstopStrength = clampValue(lowEndstopStrength, 0.0f, 1.0f);
    highEndstopStrength = clampValue(highEndstopStrength, 0.0f, 1.0f);

    if (lowEndstopPosition >= highEndstopPosition) {
      lowEndstopPosition = -1.0f;
      highEndstopPosition = 1.0f;
    }

    if (!isValidHexColor(meterColor)) {
      meterColor = "#00D26A";
    }
  }
};

struct DebugTuning {
  float detentStrengthMaxVPerRad = 2.0f;
  float snapStrengthMaxVPerRad = 2.0f;
  float clickPulseVoltage = 1.2f;
  uint32_t clickPulseMs = 34;
  float endstopMinPos = -1.0f;
  float endstopMaxPos = 1.0f;
  float endstopMinStrength = 0.70f;
  float endstopMaxStrength = 0.70f;
  bool debugStreamEnabled = false;
  uint32_t debugStreamIntervalMs = 150;

  void normalize() {
    detentStrengthMaxVPerRad = clampValue(detentStrengthMaxVPerRad, 0.0f, 12.0f);
    snapStrengthMaxVPerRad = clampValue(snapStrengthMaxVPerRad, 0.0f, 12.0f);
    clickPulseVoltage = clampValue(clickPulseVoltage, 0.0f, 6.0f);
    clickPulseMs = clampValue(clickPulseMs, 1U, 1000U);
    endstopMinPos = clampValue(endstopMinPos, -1.0f, 1.0f);
    endstopMaxPos = clampValue(endstopMaxPos, -1.0f, 1.0f);
    endstopMinStrength = clampValue(endstopMinStrength, 0.0f, 1.0f);
    endstopMaxStrength = clampValue(endstopMaxStrength, 0.0f, 1.0f);
    debugStreamIntervalMs = clampValue(debugStreamIntervalMs, 50U, 2000U);

    if (endstopMinPos >= endstopMaxPos) {
      endstopMinPos = -1.0f;
      endstopMaxPos = 1.0f;
    }
  }
};

inline void syncEndstopsFromSettings(const NormalSettings& settings, DebugTuning& tuning) {
  tuning.endstopMinPos = settings.lowEndstopPosition;
  tuning.endstopMaxPos = settings.highEndstopPosition;
  tuning.endstopMinStrength = settings.lowEndstopStrength;
  tuning.endstopMaxStrength = settings.highEndstopStrength;
  tuning.normalize();
}

inline void syncSettingsFromDebug(const DebugTuning& tuning, NormalSettings& settings) {
  settings.lowEndstopPosition = tuning.endstopMinPos;
  settings.highEndstopPosition = tuning.endstopMaxPos;
  settings.lowEndstopStrength = tuning.endstopMinStrength;
  settings.highEndstopStrength = tuning.endstopMaxStrength;
  settings.normalize();
}

}  // namespace vp
