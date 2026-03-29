#include "led/LedController.h"

#include <FastLED.h>

#include "util/MathUtil.h"

namespace vp {

namespace {
CRGB gLeds[VP_LED_TOTAL_COUNT];
}

void LedController::begin(const DeviceConfig& config, const DeviceRuntimeSettings& settings) {
  config_ = config;
  settings_ = settings;
  settings_.clamp();

  FastLED.addLeds<WS2812B, VP_PIN_LED_DATA, GRB>(gLeds, VP_LED_TOTAL_COUNT);
  FastLED.setBrightness(static_cast<uint8_t>(settings_.ledBrightness * 255.0f));
  FastLED.clear(true);

  started_ = true;
}

void LedController::onSettingsChanged(const DeviceRuntimeSettings& settings) {
  settings_ = settings;
  settings_.clamp();
}

void LedController::setKeyPressed(uint8_t keyIndex, bool pressed) {
  if (keyIndex >= 3) {
    return;
  }
  keyPressed_[keyIndex] = pressed;
}

void LedController::applyMeter(const MeterModel& meter) {
  meter_ = meter;
  meter_.rms = clampValue(meter_.rms, 0.0f, 1.0f);
  meter_.peak = clampValue(meter_.peak, 0.0f, 1.0f);
  meter_.brightness = clampValue(meter_.brightness, 0.0f, 1.0f);
}

void LedController::tick(uint32_t nowMs) {
  if (!started_) {
    return;
  }

  if ((nowMs - lastShowMs_) < 16) {
    return;
  }
  lastShowMs_ = nowMs;

  const int keyCount = config_.hardware.keyLedCount;
  const int ringCount = config_.hardware.ringLedCount;

  for (int i = 0; i < keyCount && i < 3; ++i) {
    gLeds[i] = keyPressed_[i] ? CRGB(20, 80, 180) : CRGB(8, 8, 8);
  }

  const int ringStart = keyCount;
  const int litRms = static_cast<int>(roundf(meter_.rms * static_cast<float>(ringCount)));
  const int litPeak = static_cast<int>(roundf(meter_.peak * static_cast<float>(ringCount)));

  for (int i = 0; i < ringCount; ++i) {
    const int ledIndex = ringStart + i;
    if (ledIndex >= VP_LED_TOTAL_COUNT) {
      break;
    }

    if (meter_.muted) {
      gLeds[ledIndex] = (i < litPeak) ? CRGB(80, 0, 0) : CRGB(10, 0, 0);
      continue;
    }

    if (i < litRms) {
      gLeds[ledIndex] = CRGB(0, 100, 30);
    } else {
      gLeds[ledIndex] = CRGB(0, 4, 2);
    }

    if (i == litPeak - 1) {
      gLeds[ledIndex] = CRGB(220, 210, 80);
    }
  }

  const float combinedBrightness = settings_.ledBrightness * meter_.brightness;
  FastLED.setBrightness(static_cast<uint8_t>(clampValue(combinedBrightness, 0.0f, 1.0f) * 255.0f));
  FastLED.show();
}

}  // namespace vp