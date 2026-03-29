#pragma once

#include <Arduino.h>

namespace vp {

constexpr float kTwoPi = 6.28318530718f;

template <typename T>
T clampValue(T value, T minValue, T maxValue) {
  if (value < minValue) {
    return minValue;
  }
  if (value > maxValue) {
    return maxValue;
  }
  return value;
}

inline float shortestAngleError(float target, float angle) {
  float e = target - angle;
  while (e > PI) {
    e -= kTwoPi;
  }
  while (e < -PI) {
    e += kTwoPi;
  }
  return e;
}

}  // namespace vp