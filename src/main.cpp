#include <Arduino.h>

#include "config/AppConfig.h"
#include "config/SettingsModel.h"
#include "haptics/HapticsController.h"
#include "input/KeyInput.h"
#include "led/LedRenderer.h"
#include "persistence/SettingsStore.h"
#include "protocol/ProtocolServer.h"

namespace vp {

static const DeviceConfig& gConfig = getDeviceConfig();
static NormalSettings gSettings{};
static DebugTuning gDebugTuning{};
static SettingsStore gStore{};

static HapticsController gHaptics{};
static KeyInput gKeys{};
static LedRenderer gLeds{};

static ProtocolServer* gProtocol = nullptr;

}  // namespace vp

void setup() {
  Serial.begin(115200);
  delay(250);

  vp::gSettings.normalize();
  vp::gDebugTuning.normalize();

  vp::gStore.begin();
  vp::gStore.load(vp::gSettings, vp::gDebugTuning);

  const bool hapticsReady = vp::gHaptics.begin(vp::gConfig, vp::gSettings, vp::gDebugTuning);
  vp::gKeys.begin(vp::gConfig);
  vp::gLeds.begin(vp::gConfig, vp::gSettings);

  static vp::ProtocolServer protocol(
      vp::gConfig,
      vp::gSettings,
      vp::gDebugTuning,
      vp::gHaptics,
      vp::gLeds,
      vp::gStore);
  vp::gProtocol = &protocol;
  vp::gProtocol->begin();

  if (!hapticsReady) {
    vp::gProtocol->sendDiagnosticsEvent("error", "AS5600 encoder init failed.");
  } else {
    vp::gProtocol->sendDiagnosticsEvent("info", "Encoder-only mode active (motor output disabled).");
  }
}

void loop() {
  static uint32_t lastInputMs = 0;
  static uint32_t lastDiagMs = 0;

  const uint32_t nowMs = millis();

  vp::gProtocol->tick();
  vp::gHaptics.tick(nowMs);

  const int delta = vp::gHaptics.consumeDeltaSteps();
  if (delta != 0) {
    vp::gProtocol->sendEncoderEvent(delta, vp::gHaptics.position());
  }

  if ((nowMs - lastInputMs) >= 2U) {
    lastInputMs = nowMs;

    vp::KeyPressEvent events[3];
    const size_t count = vp::gKeys.poll(events, 3, nowMs);
    for (size_t i = 0; i < count; ++i) {
      vp::gProtocol->sendButtonPressEvent(events[i].buttonId);
    }
  }

  vp::gLeds.tick(nowMs);

  if ((nowMs - lastDiagMs) >= 1000U) {
    lastDiagMs = nowMs;
    if (vp::gHaptics.diagActive()) {
      vp::gProtocol->sendDiagnosticsEvent("warning", "TMC6300 DIAG active.");
    }
  }
}
