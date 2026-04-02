#include "led/LedRenderer.h"

#include "util/ColorUtil.h"
#include "util/MathUtil.h"

namespace vp {

void LedRenderer::begin(const DeviceConfig& config, const NormalSettings& settings) {
  config_ = config;
  settings_ = settings;
  settings_.normalize();

  FastLED.addLeds<WS2812B, kPinLedData, GRB>(leds_, kTotalLedCount);
  FastLED.clear(true);
  FastLED.setBrightness(static_cast<uint8_t>(settings_.ledBrightness * 255.0f));

  started_ = true;
}

void LedRenderer::applySettings(const NormalSettings& settings) {
  settings_ = settings;
  settings_.normalize();
}

bool LedRenderer::applyMeterFrame(const MeterFrame& frame, uint32_t nowMs) {
  if (hasMeterSeq_ && frame.seq <= lastMeterSeq_) {
    return false;
  }
  hasMeterSeq_ = true;
  lastMeterSeq_ = frame.seq;

  meterFrame_ = frame;
  meterFrame_.peak = clampValue(meterFrame_.peak, 0.0f, 1.0f);
  meterFrame_.rms = clampValue(meterFrame_.rms, 0.0f, 1.0f);
  meterFrame_.brightness = clampValue(meterFrame_.brightness, 0.0f, 1.0f);
  meterFrame_.smoothing = clampValue(meterFrame_.smoothing, 0.0f, 1.0f);
  meterFrame_.peakHoldMs = clampValue(meterFrame_.peakHoldMs, 0U, 3000U);
  meterFrame_.muteRedDurationMs = clampValue(meterFrame_.muteRedDurationMs, 50U, 3000U);
  if (!isValidHexColor(meterFrame_.color)) {
    meterFrame_.color = settings_.meterColor;
  }

  if (!lastMuted_ && meterFrame_.muted) {
    setMuteOverride("#FF0000", meterFrame_.muteRedDurationMs, nowMs);
  }
  lastMuted_ = meterFrame_.muted;

  return true;
}

bool LedRenderer::setRingLed(uint8_t index, const String& colorHex, float brightness) {
  if (index >= kRingLedCount) {
    return false;
  }
  directOverrideSet_[index] = true;
  directOverrideColors_[index] = scaleColor(parseHexColor(colorHex, CRGB::Black), brightness);
  return true;
}

void LedRenderer::beginStream(const String& streamId, uint8_t ledCount) {
  streamActive_ = true;
  streamId_ = streamId;
  streamLastSeq_ = 0;
  streamBrightness_ = 1.0f;
  clearRing(streamRing_);

  if (ledCount < kRingLedCount) {
    for (uint8_t i = ledCount; i < kRingLedCount; ++i) {
      streamRing_[i] = CRGB::Black;
    }
  }
}

bool LedRenderer::applyStreamFrame(
    const String& streamId,
    uint32_t seq,
    const StreamPixelUpdate* updates,
    size_t updateCount,
    float brightness) {
  if (!streamActive_ || streamId != streamId_) {
    return false;
  }
  if (seq <= streamLastSeq_) {
    return false;
  }

  streamLastSeq_ = seq;
  streamBrightness_ = clampValue(brightness, 0.0f, 1.0f);

  for (size_t i = 0; i < updateCount; ++i) {
    if (updates[i].index >= kRingLedCount) {
      continue;
    }
    streamRing_[updates[i].index] = parseHexColor(updates[i].color, CRGB::Black);
  }

  return true;
}

void LedRenderer::endStream(const String& streamId) {
  if (!streamActive_ || streamId != streamId_) {
    return;
  }
  streamActive_ = false;
  streamId_ = "";
  streamLastSeq_ = 0;
}

void LedRenderer::setMuteOverride(const String& colorHex, uint32_t durationMs, uint32_t nowMs) {
  muteOverrideColor_ = parseHexColor(colorHex, CRGB::Red);
  muteOverrideUntilMs_ = nowMs + clampValue(durationMs, 1U, 3000U);
  muteOverrideActive_ = true;
}

bool LedRenderer::setButtonLed(uint8_t buttonId, const String& colorHex, float brightness) {
  if (buttonId == 0 || buttonId > kButtonLedCount) {
    return false;
  }

  const uint8_t index = static_cast<uint8_t>(buttonId - 1U);
  buttonColors_[index] = parseHexColor(colorHex, CRGB::Black);
  buttonBrightness_[index] = clampValue(brightness, 0.0f, 1.0f);
  return true;
}

void LedRenderer::tick(uint32_t nowMs) {
  if (!started_) {
    return;
  }

  if ((nowMs - lastShowMs_) < 16U) {
    return;
  }
  lastShowMs_ = nowMs;

  if (muteOverrideActive_ && (nowMs >= muteOverrideUntilMs_)) {
    muteOverrideActive_ = false;
  }

  renderButtons();

  CRGB ring[kRingLedCount]{};
  if (muteOverrideActive_) {
    for (uint8_t i = 0; i < kRingLedCount; ++i) {
      ring[i] = muteOverrideColor_;
    }
  } else if (streamActive_) {
    for (uint8_t i = 0; i < kRingLedCount; ++i) {
      ring[i] = scaleColor(streamRing_[i], streamBrightness_);
    }
  } else {
    renderRingFromMeter(ring, nowMs);
    for (uint8_t i = 0; i < kRingLedCount; ++i) {
      if (directOverrideSet_[i]) {
        ring[i] = directOverrideColors_[i];
      }
    }
  }

  for (uint8_t i = 0; i < kRingLedCount; ++i) {
    leds_[kButtonLedCount + i] = ring[i];
  }

  FastLED.setBrightness(static_cast<uint8_t>(clampValue(settings_.ledBrightness, 0.0f, 1.0f) * 255.0f));
  FastLED.show();
}

void LedRenderer::renderButtons() {
  for (uint8_t i = 0; i < kButtonLedCount; ++i) {
    leds_[i] = scaleColor(buttonColors_[i], buttonBrightness_[i]);
  }
}

void LedRenderer::renderRingFromMeter(CRGB* ring, uint32_t nowMs) {
  clearRing(ring);

  const float alpha = meterFrame_.smoothing;
  if (alpha <= 0.0001f) {
    smoothedRms_ = meterFrame_.rms;
    smoothedPeak_ = meterFrame_.peak;
  } else {
    smoothedRms_ += (meterFrame_.rms - smoothedRms_) * alpha;
    smoothedPeak_ += (meterFrame_.peak - smoothedPeak_) * alpha;
  }

  if (smoothedPeak_ >= heldPeak_) {
    heldPeak_ = smoothedPeak_;
    heldPeakAtMs_ = nowMs;
  } else if ((nowMs - heldPeakAtMs_) >= meterFrame_.peakHoldMs) {
    heldPeak_ = smoothedPeak_;
    heldPeakAtMs_ = nowMs;
  }

  const CRGB baseColor = parseHexColor(meterFrame_.color, CRGB(0, 210, 106));
  const CRGB meterColor = scaleColor(baseColor, meterFrame_.brightness);
  const int rmsIndex = static_cast<int>(roundf(smoothedRms_ * static_cast<float>(kRingLedCount)));
  const int peakIndex = static_cast<int>(roundf(heldPeak_ * static_cast<float>(kRingLedCount))) - 1;

  if (meterFrame_.muted) {
    for (uint8_t i = 0; i < kRingLedCount; ++i) {
      ring[i] = CRGB(40, 0, 0);
    }
    return;
  }

  if (meterFrame_.mode == MeterMode::PeakIndicator) {
    if (peakIndex >= 0 && peakIndex < static_cast<int>(kRingLedCount)) {
      ring[peakIndex] = meterColor;
    }
    return;
  }

  for (int i = 0; i < rmsIndex && i < static_cast<int>(kRingLedCount); ++i) {
    ring[i] = meterColor;
  }

  if (meterFrame_.mode == MeterMode::VuPeakHold || meterFrame_.mode == MeterMode::RingFill) {
    if (peakIndex >= 0 && peakIndex < static_cast<int>(kRingLedCount)) {
      ring[peakIndex] = CRGB(255, 255, 255);
    }
  }
}

void LedRenderer::clearRing(CRGB* ring) {
  for (uint8_t i = 0; i < kRingLedCount; ++i) {
    ring[i] = CRGB::Black;
  }
}

}  // namespace vp
