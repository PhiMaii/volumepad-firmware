#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "config/AppConfig.h"
#include "config/SettingsModel.h"
#include "haptics/HapticsController.h"
#include "led/LedRenderer.h"
#include "persistence/SettingsStore.h"

namespace vp {

class ProtocolServer {
 public:
  ProtocolServer(
      const DeviceConfig& config,
      NormalSettings& settings,
      DebugTuning& tuning,
      HapticsController& haptics,
      LedRenderer& leds,
      SettingsStore& store);

  void begin();
  void tick();

  void sendButtonPressEvent(uint8_t buttonId);
  void sendEncoderEvent(int deltaSteps, int position);
  void sendDebugStateEvent(const char* source);
  void sendDiagnosticsEvent(const char* level, const String& message);

 private:
  void processLine(const String& line);

  void handleApplySettings(JsonObjectConst payload, const char* id, const char* name);
  void handleMeterFrame(JsonObjectConst payload, const char* id, const char* name);
  void handleRingSetLed(JsonObjectConst payload, const char* id, const char* name);
  void handleRingStreamBegin(JsonObjectConst payload, const char* id, const char* name);
  void handleRingStreamFrame(JsonObjectConst payload, const char* id, const char* name);
  void handleRingStreamEnd(JsonObjectConst payload, const char* id, const char* name);
  void handleRingMuteOverride(JsonObjectConst payload, const char* id, const char* name);
  void handleButtonLedsSet(JsonObjectConst payload, const char* id, const char* name);
  void handleDebugGetState(const char* id, const char* name);
  void handleDebugApplyTuning(JsonObjectConst payload, const char* id, const char* name);
  void handleDebugSetStream(JsonObjectConst payload, const char* id, const char* name);

  void sendHelloEvent();
  void sendOkResponse(const char* id, const char* name);
  void sendOkResponseAccepted(const char* id, const char* name, bool accepted);
  void sendErrorResponse(const char* id, const char* name, const char* code, const char* message);
  void writeSettingsJson(JsonObject out, const NormalSettings& settings) const;
  void writeDebugStateJson(JsonObject out, const char* source);

  const DeviceConfig& config_;
  NormalSettings& settings_;
  DebugTuning& tuning_;
  HapticsController& haptics_;
  LedRenderer& leds_;
  SettingsStore& store_;

  static constexpr size_t kRxBufferSize = 2048;
  char rxBuffer_[kRxBufferSize]{};
  size_t rxLen_ = 0;

  uint32_t lastDebugStreamMs_ = 0;
  uint32_t inputEventSeq_ = 0;
  uint32_t debugEventSeq_ = 0;
};

}  // namespace vp
