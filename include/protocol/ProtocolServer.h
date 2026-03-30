#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "config/AppConfig.h"
#include "config/RuntimeSettings.h"
#include "display/DisplayManager.h"
#include "input/StrainInput.h"
#include "led/LedController.h"
#include "motor/HapticsController.h"

namespace vp {

class ProtocolServer {
 public:
  ProtocolServer(const DeviceConfig& config,
                 DeviceRuntimeSettings& settings,
                 HapticsController& haptics,
                 StrainInput& strain,
                 LedController& leds,
                 DisplayManager& display);

  void begin();
  void tick();

  void sendRotateEvent(int delta, int position);
  void sendPressEvent(const char* controlId, bool pressed, int position);
  void sendDiag(const char* level, const String& message);

 private:
  void processLine(const String& line);

  void handleHello(const char* requestId);
  void handleSettingsApply(JsonVariantConst payload, const char* requestId);
  void handleDisplayRender(JsonVariantConst payload, const char* requestId);
  void handleLedsMeter(JsonVariantConst payload, const char* requestId);
  void handleDebugStateGet(const char* requestId);
  void handleDebugCalibrateStrain(const char* requestId);
  void handleDebugTuningApply(JsonVariantConst payload, const char* requestId);
  void handleDebugStreamSet(JsonVariantConst payload, const char* requestId);

  void sendHello();
  void sendCapabilities();

  void sendAck(const char* requestId, const char* forType);
  void sendSettingsAck(const char* requestId);
  void sendNack(const char* requestId, const char* forType, const char* reason);
  void sendDebugState(const char* source);

  const DeviceConfig& config_;
  DeviceRuntimeSettings& settings_;
  HapticsController& haptics_;
  StrainInput& strain_;
  LedController& leds_;
  DisplayManager& display_;

  char rxBuffer_[VP_MAX_PACKET_SIZE * 2]{};
  size_t rxLen_ = 0;

  bool debugStreamEnabled_ = false;
  uint32_t debugStreamIntervalMs_ = 150;
  uint32_t lastDebugStreamMs_ = 0;
};

}  // namespace vp