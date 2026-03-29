#include "display/St7789Display.h"

#include "util/MathUtil.h"

namespace vp {

void St7789Display::begin(const DeviceConfig& config) {
  config_ = config;
  started_ = true;
}

void St7789Display::setBrightness(float brightness) {
  if (!started_ || config_.pins.tftBl < 0) {
    return;
  }

  const int pwm = static_cast<int>(clampValue(brightness, 0.0f, 1.0f) * 255.0f);
  analogWrite(config_.pins.tftBl, pwm);
}

void St7789Display::render(const DisplayModel& model) {
  (void)model;
  // Placeholder for final ST7789 render path.
}

}  // namespace vp