#include <Arduino.h>
#include <cstdio>

#include "config/AppConfig.h"
#include "config/RuntimeSettings.h"
#include "display/DisplayManager.h"
#include "input/KeyInput.h"
#include "input/StrainInput.h"
#include "led/LedController.h"
#include "motor/HapticsController.h"
#include "protocol/ProtocolServer.h"
#include "sensors/AngleSensorManager.h"

namespace vp {

static const DeviceConfig& gConfig = getDeviceConfig();
static DeviceRuntimeSettings gSettings{};

static AngleSensorManager gSensorManager;
static HapticsController gHaptics;
static KeyInput gKeyInput;
static StrainInput gStrainInput;
static LedController gLedController;
static DisplayManager gDisplayManager;

static ProtocolServer* gProtocol = nullptr;

}  // namespace vp

void setup() {
  Serial.begin(115200);
  delay(300);

  vp::gSettings.clamp();

  vp::gSensorManager.begin(vp::gConfig);
  const bool hapticsReady = vp::gHaptics.begin(vp::gConfig, vp::gSensorManager.sensor());

  vp::gKeyInput.begin(vp::gConfig);
  const bool strainReady = vp::gStrainInput.begin(vp::gConfig);

  vp::gLedController.begin(vp::gConfig, vp::gSettings);
  vp::gDisplayManager.begin(vp::gConfig, vp::gSettings);

  static vp::ProtocolServer protocol(vp::gConfig, vp::gSettings, vp::gHaptics, vp::gLedController, vp::gDisplayManager);
  vp::gProtocol = &protocol;
  vp::gProtocol->begin();

  if (!hapticsReady) {
    vp::gProtocol->sendDiag("error", "Haptics init failed");
  }

  if (!strainReady) {
    vp::gProtocol->sendDiag("warning", "HX711 not ready or pins invalid");
  }

  if (vp::gConfig.sensorBackend == vp::AngleSensorBackend::As5600I2c) {
    vp::gProtocol->sendDiag("info", "Angle sensor backend: AS5600 I2C");
  } else {
    vp::gProtocol->sendDiag("info", "Angle sensor backend: MT6701 SPI");
  }
}

void loop() {
  static uint32_t lastInputScanMs = 0;
  static uint32_t lastDiagMs = 0;

  const uint32_t nowMs = millis();

  vp::gProtocol->tick();
  vp::gHaptics.tick(nowMs);

  const int delta = vp::gHaptics.consumeDetentDelta();
  if (delta != 0) {
    vp::gProtocol->sendRotateEvent(delta, vp::gHaptics.currentPosition());
  }

  if ((nowMs - lastInputScanMs) >= 2U) {
    lastInputScanMs = nowMs;

    vp::KeyEvent keyEvents[3];
    const size_t keyCount = vp::gKeyInput.poll(keyEvents, 3, nowMs);
    for (size_t i = 0; i < keyCount; ++i) {
      char controlId[16];
      snprintf(controlId, sizeof(controlId), "button-%u", static_cast<unsigned>(keyEvents[i].keyIndex + 1U));
      vp::gProtocol->sendPressEvent(controlId, keyEvents[i].pressed, vp::gHaptics.currentPosition());
      vp::gLedController.setKeyPressed(keyEvents[i].keyIndex, keyEvents[i].pressed);
    }

    const vp::StrainEvent strainEvent = vp::gStrainInput.poll(nowMs);
    if (strainEvent.changed) {
      if (strainEvent.triggerClick) {
        vp::gHaptics.triggerClickPulse(nowMs);
      }
      vp::gProtocol->sendPressEvent("encoder-main", strainEvent.pressed, vp::gHaptics.currentPosition());
    }
  }

  vp::gLedController.tick(nowMs);
  vp::gDisplayManager.tick(nowMs);

  if ((nowMs - lastDiagMs) >= 1000U) {
    lastDiagMs = nowMs;
    if (vp::gConfig.pins.diag >= 0 && digitalRead(vp::gConfig.pins.diag) == LOW) {
      vp::gProtocol->sendDiag("warning", "TMC6300 DIAG active");
    }
  }
}