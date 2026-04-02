#pragma once

#include <Arduino.h>
#include <FastLED.h>

#include "util/MathUtil.h"

namespace vp {

inline bool isValidHexColor(const String& value) {
  if (value.length() != 7 || value[0] != '#') {
    return false;
  }
  for (int i = 1; i < 7; ++i) {
    const char c = value[i];
    const bool isDigit = (c >= '0' && c <= '9');
    const bool isLowerHex = (c >= 'a' && c <= 'f');
    const bool isUpperHex = (c >= 'A' && c <= 'F');
    if (!isDigit && !isLowerHex && !isUpperHex) {
      return false;
    }
  }
  return true;
}

inline uint8_t parseHexByte(const String& value, int startIndex) {
  const String slice = value.substring(startIndex, startIndex + 2);
  return static_cast<uint8_t>(strtoul(slice.c_str(), nullptr, 16));
}

inline CRGB parseHexColor(const String& value, const CRGB& fallback = CRGB::Black) {
  if (!isValidHexColor(value)) {
    return fallback;
  }
  return CRGB(parseHexByte(value, 1), parseHexByte(value, 3), parseHexByte(value, 5));
}

inline String colorToHex(const CRGB& color) {
  char buffer[8];
  snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", color.r, color.g, color.b);
  return String(buffer);
}

inline CRGB scaleColor(const CRGB& color, float brightness) {
  const float clamped = clampValue(brightness, 0.0f, 1.0f);
  return CRGB(
      static_cast<uint8_t>(static_cast<float>(color.r) * clamped),
      static_cast<uint8_t>(static_cast<float>(color.g) * clamped),
      static_cast<uint8_t>(static_cast<float>(color.b) * clamped));
}

}  // namespace vp
