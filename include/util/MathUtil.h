#pragma once

#include <Arduino.h>

namespace vp {

constexpr float kTwoPi = 6.28318530718f;

template <typename T>
inline T clampValue(T value, T minValue, T maxValue) {
  if (value < minValue) {
    return minValue;
  }
  if (value > maxValue) {
    return maxValue;
  }
  return value;
}

inline float shortestAngleError(float target, float angle) {
  float error = target - angle;
  while (error > PI) {
    error -= kTwoPi;
  }
  while (error < -PI) {
    error += kTwoPi;
  }
  return error;
}

inline float mapNormalizedToRadians(float normalized) {
  return normalized * kTwoPi * 0.5f;
}

}  // namespace vp
