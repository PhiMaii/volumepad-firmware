#pragma once

#include <Arduino.h>
#include <FastLED.h>

#include "config/AppConfig.h"
#include "config/SettingsModel.h"

namespace vp {

struct MeterFrame {
  uint32_t seq = 0;
  float peak = 0.0f;
  float rms = 0.0f;
  bool muted = false;
  MeterMode mode = MeterMode::RingFill;
  String color = "#00D26A";
  float brightness = 0.80f;
  float smoothing = 0.25f;
  uint32_t peakHoldMs = 500;
  uint32_t muteRedDurationMs = 700;
};

struct StreamPixelUpdate {
  uint8_t index = 0;
  String color = "#000000";
};

class LedRenderer {
 public:
  void begin(const DeviceConfig& config, const NormalSettings& settings);
  void applySettings(const NormalSettings& settings);

  bool applyMeterFrame(const MeterFrame& frame, uint32_t nowMs);
  bool setRingLed(uint8_t index, const String& colorHex, float brightness);
  void beginStream(const String& streamId, uint8_t ledCount);
  bool applyStreamFrame(
      const String& streamId,
      uint32_t seq,
      const StreamPixelUpdate* updates,
      size_t updateCount,
      float brightness);
  void endStream(const String& streamId);
  void setMuteOverride(const String& colorHex, uint32_t durationMs, uint32_t nowMs);
  bool setButtonLed(uint8_t buttonId, const String& colorHex, float brightness);

  void tick(uint32_t nowMs);

 private:
  void renderButtons();
  void renderRingFromMeter(CRGB* ring, uint32_t nowMs);
  void clearRing(CRGB* ring);

  DeviceConfig config_{};
  NormalSettings settings_{};

  bool started_ = false;
  uint32_t lastShowMs_ = 0;

  CRGB leds_[kTotalLedCount]{};

  CRGB buttonColors_[kButtonLedCount] = {CRGB(24, 24, 24), CRGB(24, 24, 24), CRGB(24, 24, 24)};
  float buttonBrightness_[kButtonLedCount] = {0.20f, 0.20f, 0.20f};

  MeterFrame meterFrame_{};
  bool hasMeterSeq_ = false;
  uint32_t lastMeterSeq_ = 0;
  bool lastMuted_ = false;
  float smoothedRms_ = 0.0f;
  float smoothedPeak_ = 0.0f;
  float heldPeak_ = 0.0f;
  uint32_t heldPeakAtMs_ = 0;

  bool streamActive_ = false;
  String streamId_{};
  uint32_t streamLastSeq_ = 0;
  float streamBrightness_ = 1.0f;
  CRGB streamRing_[kRingLedCount]{};

  bool muteOverrideActive_ = false;
  CRGB muteOverrideColor_ = CRGB::Red;
  uint32_t muteOverrideUntilMs_ = 0;

  bool directOverrideSet_[kRingLedCount] = {};
  CRGB directOverrideColors_[kRingLedCount]{};
};

}  // namespace vp
