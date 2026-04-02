#include "protocol/ProtocolServer.h"

#include "util/ColorUtil.h"
#include "util/MathUtil.h"

namespace vp {

namespace {

bool readBoolField(JsonObjectConst obj, const char* key, bool& outValue) {
  if (!obj.containsKey(key)) {
    return true;
  }
  if (!obj[key].is<bool>()) {
    return false;
  }
  outValue = obj[key].as<bool>();
  return true;
}

bool readIntField(JsonObjectConst obj, const char* key, int& outValue) {
  if (!obj.containsKey(key)) {
    return true;
  }
  if (!obj[key].is<int>()) {
    return false;
  }
  outValue = obj[key].as<int>();
  return true;
}

bool readUIntField(JsonObjectConst obj, const char* key, uint32_t& outValue) {
  if (!obj.containsKey(key)) {
    return true;
  }
  if (!obj[key].is<uint32_t>() && !obj[key].is<int>()) {
    return false;
  }
  const int raw = obj[key].as<int>();
  if (raw < 0) {
    return false;
  }
  outValue = static_cast<uint32_t>(raw);
  return true;
}

bool readFloatField(JsonObjectConst obj, const char* key, float& outValue) {
  if (!obj.containsKey(key)) {
    return true;
  }
  if (!obj[key].is<float>() && !obj[key].is<int>()) {
    return false;
  }
  outValue = obj[key].as<float>();
  return true;
}

bool readStringField(JsonObjectConst obj, const char* key, String& outValue) {
  if (!obj.containsKey(key)) {
    return true;
  }
  if (!obj[key].is<const char*>()) {
    return false;
  }
  outValue = obj[key].as<const char*>();
  return true;
}

}  // namespace

ProtocolServer::ProtocolServer(
    const DeviceConfig& config,
    NormalSettings& settings,
    DebugTuning& tuning,
    HapticsController& haptics,
    LedRenderer& leds,
    SettingsStore& store)
    : config_(config), settings_(settings), tuning_(tuning), haptics_(haptics), leds_(leds), store_(store) {}

void ProtocolServer::begin() {
  sendHelloEvent();
}

void ProtocolServer::tick() {
  while (Serial.available() > 0) {
    const int value = Serial.read();
    if (value < 0) {
      return;
    }

    const char c = static_cast<char>(value);
    if (c == '\n') {
      if (rxLen_ == 0) {
        continue;
      }

      if (rxBuffer_[rxLen_ - 1] == '\r') {
        rxBuffer_[rxLen_ - 1] = '\0';
      } else {
        rxBuffer_[rxLen_] = '\0';
      }

      processLine(String(rxBuffer_));
      rxLen_ = 0;
      continue;
    }

    if (rxLen_ >= (kRxBufferSize - 1)) {
      rxLen_ = 0;
      continue;
    }

    rxBuffer_[rxLen_++] = c;
  }

  const uint32_t nowMs = millis();
  if (tuning_.debugStreamEnabled && (nowMs - lastDebugStreamMs_) >= tuning_.debugStreamIntervalMs) {
    lastDebugStreamMs_ = nowMs;
    sendDebugStateEvent("stream");
  }
}

void ProtocolServer::sendButtonPressEvent(uint8_t buttonId) {
  StaticJsonDocument<256> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "event";
  doc["name"] = "device.input.button";
  JsonObject payload = doc.createNestedObject("payload");
  payload["buttonId"] = buttonId;
  payload["action"] = "press";
  payload["uptimeMs"] = millis();
  payload["eventSeq"] = ++inputEventSeq_;
  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendEncoderEvent(int deltaSteps, int position) {
  if (deltaSteps == 0) {
    return;
  }

  StaticJsonDocument<320> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "event";
  doc["name"] = "device.input.encoder";
  JsonObject payload = doc.createNestedObject("payload");
  payload["deltaSteps"] = deltaSteps;
  payload["pressed"] = false;
  payload["position"] = position;
  payload["uptimeMs"] = millis();
  payload["eventSeq"] = ++inputEventSeq_;
  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendDebugStateEvent(const char* source) {
  StaticJsonDocument<768> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "event";
  doc["name"] = "debug.state";
  JsonObject payload = doc.createNestedObject("payload");
  writeDebugStateJson(payload, source);
  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendDiagnosticsEvent(const char* level, const String& message) {
  StaticJsonDocument<384> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "event";
  doc["name"] = "event.diagnostics";
  JsonObject payload = doc.createNestedObject("payload");
  payload["level"] = level;
  payload["message"] = message;
  payload["uptimeMs"] = millis();
  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::processLine(const String& line) {
  if (line.length() == 0) {
    return;
  }

  DynamicJsonDocument doc(4096);
  const DeserializationError error = deserializeJson(doc, line);
  if (error) {
    return;
  }

  const int version = doc["v"] | 0;
  const char* type = doc["type"] | "";
  const char* id = doc["id"] | nullptr;
  const char* name = doc["name"] | "";

  if (version != kProtocolVersion || strcmp(type, "request") != 0 || id == nullptr || strlen(name) == 0) {
    sendErrorResponse(id != nullptr ? id : "", strlen(name) > 0 ? name : "", "invalid_payload", "Invalid request envelope.");
    return;
  }

  if (!doc["payload"].is<JsonObject>()) {
    sendErrorResponse(id, name, "invalid_payload", "payload must be a JSON object.");
    return;
  }
  JsonObjectConst payload = doc["payload"].as<JsonObjectConst>();

  if (strcmp(name, "device.applySettings") == 0) {
    handleApplySettings(payload, id, name);
    return;
  }
  if (strcmp(name, "device.meter.frame") == 0) {
    handleMeterFrame(payload, id, name);
    return;
  }
  if (strcmp(name, "device.ring.setLed") == 0) {
    handleRingSetLed(payload, id, name);
    return;
  }
  if (strcmp(name, "device.ring.stream.begin") == 0) {
    handleRingStreamBegin(payload, id, name);
    return;
  }
  if (strcmp(name, "device.ring.stream.frame") == 0) {
    handleRingStreamFrame(payload, id, name);
    return;
  }
  if (strcmp(name, "device.ring.stream.end") == 0) {
    handleRingStreamEnd(payload, id, name);
    return;
  }
  if (strcmp(name, "device.ring.muteOverride") == 0) {
    handleRingMuteOverride(payload, id, name);
    return;
  }
  if (strcmp(name, "device.buttonLeds.set") == 0) {
    handleButtonLedsSet(payload, id, name);
    return;
  }
  if (strcmp(name, "debug.getState") == 0) {
    handleDebugGetState(id, name);
    return;
  }
  if (strcmp(name, "debug.applyTuning") == 0) {
    handleDebugApplyTuning(payload, id, name);
    return;
  }
  if (strcmp(name, "debug.setStream") == 0) {
    handleDebugSetStream(payload, id, name);
    return;
  }

  sendErrorResponse(id, name, "unknown_method", "Unsupported request name.");
}

void ProtocolServer::handleApplySettings(JsonObjectConst payload, const char* id, const char* name) {
  NormalSettings next = settings_;
  String meterMode = meterModeToString(next.meterMode);

  if (!readBoolField(payload, "autoReconnectOnError", next.autoReconnectOnError) ||
      !readBoolField(payload, "autoConnectOnStartup", next.autoConnectOnStartup) ||
      !readFloatField(payload, "volumeStepSize", next.volumeStepSize) ||
      !readIntField(payload, "detentCount", next.detentCount) ||
      !readFloatField(payload, "detentStrength", next.detentStrength) ||
      !readFloatField(payload, "snapStrength", next.snapStrength) ||
      !readBoolField(payload, "encoderInvert", next.encoderInvert) ||
      !readFloatField(payload, "ledBrightness", next.ledBrightness) ||
      !readStringField(payload, "meterMode", meterMode) ||
      !readStringField(payload, "meterColor", next.meterColor) ||
      !readFloatField(payload, "meterBrightness", next.meterBrightness) ||
      !readFloatField(payload, "meterSmoothing", next.meterSmoothing) ||
      !readUIntField(payload, "meterPeakHoldMs", next.meterPeakHoldMs) ||
      !readUIntField(payload, "meterMuteRedDurationMs", next.meterMuteRedDurationMs) ||
      !readBoolField(payload, "lowEndstopEnabled", next.lowEndstopEnabled) ||
      !readFloatField(payload, "lowEndstopPosition", next.lowEndstopPosition) ||
      !readFloatField(payload, "lowEndstopStrength", next.lowEndstopStrength) ||
      !readBoolField(payload, "highEndstopEnabled", next.highEndstopEnabled) ||
      !readFloatField(payload, "highEndstopPosition", next.highEndstopPosition) ||
      !readFloatField(payload, "highEndstopStrength", next.highEndstopStrength)) {
    sendErrorResponse(id, name, "invalid_payload", "Settings payload contains invalid field types.");
    return;
  }

  next.meterMode = parseMeterMode(meterMode);
  next.normalize();
  settings_ = next;

  syncEndstopsFromSettings(settings_, tuning_);
  tuning_.normalize();

  store_.saveSettings(settings_);
  store_.saveDebugTuning(tuning_);
  haptics_.applySettings(settings_, tuning_);
  leds_.applySettings(settings_);

  StaticJsonDocument<1024> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "response";
  doc["id"] = id;
  doc["name"] = name;
  doc["ok"] = true;
  JsonObject payloadOut = doc.createNestedObject("payload");
  JsonObject effective = payloadOut.createNestedObject("effective");
  writeSettingsJson(effective, settings_);
  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::handleMeterFrame(JsonObjectConst payload, const char* id, const char* name) {
  MeterFrame frame;
  if (!payload.containsKey("seq")) {
    sendErrorResponse(id, name, "invalid_payload", "seq is required.");
    return;
  }
  if (!readUIntField(payload, "seq", frame.seq) ||
      !readFloatField(payload, "peak", frame.peak) ||
      !readFloatField(payload, "rms", frame.rms) ||
      !readBoolField(payload, "muted", frame.muted) ||
      !readStringField(payload, "color", frame.color) ||
      !readFloatField(payload, "brightness", frame.brightness) ||
      !readFloatField(payload, "smoothing", frame.smoothing) ||
      !readUIntField(payload, "peakHoldMs", frame.peakHoldMs) ||
      !readUIntField(payload, "muteRedDurationMs", frame.muteRedDurationMs)) {
    sendErrorResponse(id, name, "invalid_payload", "Meter payload contains invalid field types.");
    return;
  }

  String mode = meterModeToString(frame.mode);
  if (!readStringField(payload, "mode", mode)) {
    sendErrorResponse(id, name, "invalid_payload", "mode must be a string.");
    return;
  }
  frame.mode = parseMeterMode(mode);
  if (!isValidHexColor(frame.color)) {
    sendErrorResponse(id, name, "invalid_payload", "color must be #RRGGBB.");
    return;
  }

  const bool accepted = leds_.applyMeterFrame(frame, millis());
  sendOkResponseAccepted(id, name, accepted);
}

void ProtocolServer::handleRingSetLed(JsonObjectConst payload, const char* id, const char* name) {
  int index = -1;
  String color = "#000000";
  float brightness = 1.0f;

  if (!readIntField(payload, "index", index) || !readStringField(payload, "color", color) ||
      !readFloatField(payload, "brightness", brightness)) {
    sendErrorResponse(id, name, "invalid_payload", "ring.setLed payload contains invalid field types.");
    return;
  }
  if (index < 0 || index >= static_cast<int>(kRingLedCount)) {
    sendErrorResponse(id, name, "out_of_range", "index must be within 0..26.");
    return;
  }
  if (!isValidHexColor(color)) {
    sendErrorResponse(id, name, "invalid_payload", "color must be #RRGGBB.");
    return;
  }

  leds_.setRingLed(static_cast<uint8_t>(index), color, brightness);
  sendOkResponse(id, name);
}

void ProtocolServer::handleRingStreamBegin(JsonObjectConst payload, const char* id, const char* name) {
  String streamId;
  int ledCount = kRingLedCount;
  if (!readStringField(payload, "streamId", streamId) || !readIntField(payload, "ledCount", ledCount) || streamId.length() == 0) {
    sendErrorResponse(id, name, "invalid_payload", "stream.begin requires streamId.");
    return;
  }
  if (ledCount != static_cast<int>(kRingLedCount)) {
    sendErrorResponse(id, name, "out_of_range", "ledCount must equal 27.");
    return;
  }
  leds_.beginStream(streamId, static_cast<uint8_t>(ledCount));
  sendOkResponse(id, name);
}

void ProtocolServer::handleRingStreamFrame(JsonObjectConst payload, const char* id, const char* name) {
  String streamId;
  uint32_t seq = 0;
  float brightness = 1.0f;

  if (!readStringField(payload, "streamId", streamId) || !readUIntField(payload, "seq", seq) ||
      !readFloatField(payload, "brightness", brightness) || !payload["leds"].is<JsonArray>()) {
    sendErrorResponse(id, name, "invalid_payload", "stream.frame requires streamId, seq, leds[].");
    return;
  }

  StreamPixelUpdate updates[kRingLedCount];
  size_t updateCount = 0;

  for (JsonVariantConst item : payload["leds"].as<JsonArrayConst>()) {
    if (!item.is<JsonObjectConst>()) {
      sendErrorResponse(id, name, "invalid_payload", "Each leds[] entry must be an object.");
      return;
    }
    JsonObjectConst ledObj = item.as<JsonObjectConst>();

    int index = -1;
    String color;
    if (!readIntField(ledObj, "index", index) || !readStringField(ledObj, "color", color)) {
      sendErrorResponse(id, name, "invalid_payload", "Each leds[] entry must include index and color.");
      return;
    }
    if (index < 0 || index >= static_cast<int>(kRingLedCount)) {
      sendErrorResponse(id, name, "out_of_range", "led index must be within 0..26.");
      return;
    }
    if (!isValidHexColor(color)) {
      sendErrorResponse(id, name, "invalid_payload", "led color must be #RRGGBB.");
      return;
    }
    if (updateCount < kRingLedCount) {
      updates[updateCount].index = static_cast<uint8_t>(index);
      updates[updateCount].color = color;
      ++updateCount;
    }
  }

  const bool accepted = leds_.applyStreamFrame(streamId, seq, updates, updateCount, brightness);
  sendOkResponseAccepted(id, name, accepted);
}

void ProtocolServer::handleRingStreamEnd(JsonObjectConst payload, const char* id, const char* name) {
  String streamId;
  if (!readStringField(payload, "streamId", streamId) || streamId.length() == 0) {
    sendErrorResponse(id, name, "invalid_payload", "stream.end requires streamId.");
    return;
  }
  leds_.endStream(streamId);
  sendOkResponse(id, name);
}

void ProtocolServer::handleRingMuteOverride(JsonObjectConst payload, const char* id, const char* name) {
  String color = "#FF0000";
  uint32_t durationMs = 700;
  if (!readStringField(payload, "color", color) || !readUIntField(payload, "durationMs", durationMs)) {
    sendErrorResponse(id, name, "invalid_payload", "muteOverride requires color and durationMs.");
    return;
  }
  if (!isValidHexColor(color)) {
    sendErrorResponse(id, name, "invalid_payload", "color must be #RRGGBB.");
    return;
  }

  leds_.setMuteOverride(color, durationMs, millis());
  sendOkResponse(id, name);
}

void ProtocolServer::handleButtonLedsSet(JsonObjectConst payload, const char* id, const char* name) {
  const char* keys[3] = {"button1", "button2", "button3"};
  for (uint8_t buttonId = 1; buttonId <= 3; ++buttonId) {
    const char* key = keys[buttonId - 1];
    if (!payload.containsKey(key)) {
      continue;
    }
    if (!payload[key].is<JsonObjectConst>()) {
      sendErrorResponse(id, name, "invalid_payload", "button payload must be an object.");
      return;
    }

    JsonObjectConst buttonObj = payload[key].as<JsonObjectConst>();
    String color = "#000000";
    float brightness = 0.0f;
    if (!readStringField(buttonObj, "color", color) || !readFloatField(buttonObj, "brightness", brightness)) {
      sendErrorResponse(id, name, "invalid_payload", "button payload requires color and brightness.");
      return;
    }
    if (!isValidHexColor(color)) {
      sendErrorResponse(id, name, "invalid_payload", "button color must be #RRGGBB.");
      return;
    }
    leds_.setButtonLed(buttonId, color, brightness);
  }
  sendOkResponse(id, name);
}

void ProtocolServer::handleDebugGetState(const char* id, const char* name) {
  StaticJsonDocument<1024> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "response";
  doc["id"] = id;
  doc["name"] = name;
  doc["ok"] = true;
  JsonObject payload = doc.createNestedObject("payload");
  writeDebugStateJson(payload, "request");
  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::handleDebugApplyTuning(JsonObjectConst payload, const char* id, const char* name) {
  DebugTuning next = tuning_;
  if (!readFloatField(payload, "detentStrengthMaxVPerRad", next.detentStrengthMaxVPerRad) ||
      !readFloatField(payload, "snapStrengthMaxVPerRad", next.snapStrengthMaxVPerRad) ||
      !readFloatField(payload, "clickPulseVoltage", next.clickPulseVoltage) ||
      !readUIntField(payload, "clickPulseMs", next.clickPulseMs) ||
      !readFloatField(payload, "endstopMinPos", next.endstopMinPos) ||
      !readFloatField(payload, "endstopMaxPos", next.endstopMaxPos) ||
      !readFloatField(payload, "endstopMinStrength", next.endstopMinStrength) ||
      !readFloatField(payload, "endstopMaxStrength", next.endstopMaxStrength)) {
    sendErrorResponse(id, name, "invalid_payload", "debug.applyTuning payload contains invalid field types.");
    return;
  }

  next.normalize();
  tuning_ = next;
  syncSettingsFromDebug(tuning_, settings_);
  settings_.normalize();

  store_.saveDebugTuning(tuning_);
  store_.saveSettings(settings_);
  haptics_.applySettings(settings_, tuning_);
  leds_.applySettings(settings_);

  sendDebugStateEvent("tuning-apply");

  StaticJsonDocument<1024> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "response";
  doc["id"] = id;
  doc["name"] = name;
  doc["ok"] = true;
  JsonObject payloadOut = doc.createNestedObject("payload");
  writeDebugStateJson(payloadOut, "tuning-apply");
  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::handleDebugSetStream(JsonObjectConst payload, const char* id, const char* name) {
  DebugTuning next = tuning_;
  if (!readBoolField(payload, "enabled", next.debugStreamEnabled) ||
      !readUIntField(payload, "intervalMs", next.debugStreamIntervalMs)) {
    sendErrorResponse(id, name, "invalid_payload", "debug.setStream payload contains invalid fields.");
    return;
  }

  next.normalize();
  tuning_ = next;
  store_.saveDebugTuning(tuning_);

  StaticJsonDocument<512> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "response";
  doc["id"] = id;
  doc["name"] = name;
  doc["ok"] = true;
  JsonObject payloadOut = doc.createNestedObject("payload");
  payloadOut["enabled"] = tuning_.debugStreamEnabled;
  payloadOut["intervalMs"] = tuning_.debugStreamIntervalMs;
  serializeJson(doc, Serial);
  Serial.print('\n');

  sendDebugStateEvent("stream-set");
}

void ProtocolServer::sendHelloEvent() {
  StaticJsonDocument<320> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "event";
  doc["name"] = "device.hello";
  JsonObject payload = doc.createNestedObject("payload");
  payload["deviceId"] = config_.deviceId;
  payload["firmwareVersion"] = config_.firmwareVersion;
  payload["ringLedCount"] = kRingLedCount;
  payload["buttonLedCount"] = kButtonLedCount;
  payload["uptimeMs"] = millis();
  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendOkResponse(const char* id, const char* name) {
  StaticJsonDocument<320> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "response";
  doc["id"] = id;
  doc["name"] = name;
  doc["ok"] = true;
  doc.createNestedObject("payload");
  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendOkResponseAccepted(const char* id, const char* name, bool accepted) {
  StaticJsonDocument<320> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "response";
  doc["id"] = id;
  doc["name"] = name;
  doc["ok"] = true;
  JsonObject payload = doc.createNestedObject("payload");
  payload["accepted"] = accepted;
  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendErrorResponse(const char* id, const char* name, const char* code, const char* message) {
  StaticJsonDocument<512> doc;
  doc["v"] = kProtocolVersion;
  doc["type"] = "response";
  doc["id"] = id;
  doc["name"] = name;
  doc["ok"] = false;
  JsonObject error = doc.createNestedObject("error");
  error["code"] = code;
  error["message"] = message;
  doc.createNestedObject("payload");
  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::writeSettingsJson(JsonObject out, const NormalSettings& settings) const {
  out["autoReconnectOnError"] = settings.autoReconnectOnError;
  out["autoConnectOnStartup"] = settings.autoConnectOnStartup;
  out["volumeStepSize"] = settings.volumeStepSize;
  out["detentCount"] = settings.detentCount;
  out["detentStrength"] = settings.detentStrength;
  out["snapStrength"] = settings.snapStrength;
  out["encoderInvert"] = settings.encoderInvert;
  out["ledBrightness"] = settings.ledBrightness;
  out["meterMode"] = meterModeToString(settings.meterMode);
  out["meterColor"] = settings.meterColor;
  out["meterBrightness"] = settings.meterBrightness;
  out["meterSmoothing"] = settings.meterSmoothing;
  out["meterPeakHoldMs"] = settings.meterPeakHoldMs;
  out["meterMuteRedDurationMs"] = settings.meterMuteRedDurationMs;
  out["lowEndstopEnabled"] = settings.lowEndstopEnabled;
  out["lowEndstopPosition"] = settings.lowEndstopPosition;
  out["lowEndstopStrength"] = settings.lowEndstopStrength;
  out["highEndstopEnabled"] = settings.highEndstopEnabled;
  out["highEndstopPosition"] = settings.highEndstopPosition;
  out["highEndstopStrength"] = settings.highEndstopStrength;
}

void ProtocolServer::writeDebugStateJson(JsonObject out, const char* source) {
  const HapticsDebugState hapticsState = haptics_.getDebugState();
  out["deviceId"] = config_.deviceId;
  out["source"] = source;
  out["uptimeMs"] = millis();
  out["eventSeq"] = ++debugEventSeq_;
  out["hapticsReady"] = hapticsState.ready;
  out["position"] = hapticsState.position;
  out["detentCount"] = settings_.detentCount;
  out["detentStrength"] = settings_.detentStrength;
  out["snapStrength"] = settings_.snapStrength;

  out["detentStrengthMaxVPerRad"] = tuning_.detentStrengthMaxVPerRad;
  out["snapStrengthMaxVPerRad"] = tuning_.snapStrengthMaxVPerRad;
  out["clickPulseVoltage"] = tuning_.clickPulseVoltage;
  out["clickPulseMs"] = tuning_.clickPulseMs;
  out["endstopMinPos"] = tuning_.endstopMinPos;
  out["endstopMaxPos"] = tuning_.endstopMaxPos;
  out["endstopMinStrength"] = tuning_.endstopMinStrength;
  out["endstopMaxStrength"] = tuning_.endstopMaxStrength;
  out["debugStreamEnabled"] = tuning_.debugStreamEnabled;
  out["debugStreamIntervalMs"] = tuning_.debugStreamIntervalMs;

  out["shaftAngle"] = hapticsState.shaftAngle;
  out["appliedVoltage"] = hapticsState.appliedVoltage;
  out["endstopMinRad"] = hapticsState.endstopMinRad;
  out["endstopMaxRad"] = hapticsState.endstopMaxRad;
}

}  // namespace vp
