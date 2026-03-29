#include "display/DisplayManager.h"

namespace vp {

void DisplayManager::begin(const DeviceConfig& config, const DeviceRuntimeSettings& settings) {
  config_ = config;
  settings_ = settings;
  settings_.clamp();

  st7789_.begin(config_);
  st7789_.setBrightness(settings_.displayBrightness);

  started_ = true;
}

void DisplayManager::onSettingsChanged(const DeviceRuntimeSettings& settings) {
  settings_ = settings;
  settings_.clamp();

  if (started_) {
    st7789_.setBrightness(settings_.displayBrightness);
  }
}

void DisplayManager::applyRenderPayload(JsonVariantConst payload) {
  if (!payload.is<JsonObjectConst>()) {
    return;
  }

  JsonObjectConst obj = payload.as<JsonObjectConst>();
  lastModel_.screen = obj["screen"] | "";
  lastModel_.title = obj["title"] | "";
  lastModel_.subtitle = obj["subtitle"] | "";
  lastModel_.valueText = obj["valueText"] | "";
  lastModel_.iconRef = obj["iconRef"] | "";
  lastModel_.muted = obj["muted"] | false;
  lastModel_.accent = obj["accent"] | "#00D26A";

  st7789_.render(lastModel_);
}

void DisplayManager::tick(uint32_t nowMs) {
  (void)nowMs;
  // Final display frame scheduling is intentionally deferred.
}

}  // namespace vp