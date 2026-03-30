#include "protocol/ProtocolServer.h"

#include <ArduinoJson.h>

#include "util/MathUtil.h"

namespace vp {

ProtocolServer::ProtocolServer(const DeviceConfig& config,
                               DeviceRuntimeSettings& settings,
                               HapticsController& haptics,
                               StrainInput& strain,
                               LedController& leds,
                               DisplayManager& display)
    : config_(config), settings_(settings), haptics_(haptics), strain_(strain), leds_(leds), display_(display) {}

void ProtocolServer::begin() {
  sendHello();
  sendCapabilities();
}

void ProtocolServer::tick() {
  while (Serial.available() > 0) {
    const int readByte = Serial.read();
    if (readByte < 0) {
      return;
    }

    const char c = static_cast<char>(readByte);
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

    if (rxLen_ >= (sizeof(rxBuffer_) - 1)) {
      rxLen_ = 0;
      continue;
    }

    rxBuffer_[rxLen_++] = c;
  }

  const uint32_t nowMs = millis();
  if (debugStreamEnabled_ && (nowMs - lastDebugStreamMs_) >= debugStreamIntervalMs_) {
    lastDebugStreamMs_ = nowMs;
    sendDebugState("stream");
  }
}

void ProtocolServer::sendRotateEvent(int delta, int position) {
  if (delta == 0) {
    return;
  }

  StaticJsonDocument<256> doc;
  doc["type"] = "input.event";
  JsonObject payload = doc.createNestedObject("payload");
  payload["deviceId"] = config_.identity.deviceId;
  payload["controlId"] = "encoder-main";
  payload["eventType"] = "rotate";
  payload["delta"] = delta;
  payload["pressed"] = false;
  payload["position"] = position;

  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendPressEvent(const char* controlId, bool pressed, int position) {
  StaticJsonDocument<256> doc;
  doc["type"] = "input.event";
  JsonObject payload = doc.createNestedObject("payload");
  payload["deviceId"] = config_.identity.deviceId;
  payload["controlId"] = controlId;
  payload["eventType"] = "press";
  payload["delta"] = 0;
  payload["pressed"] = pressed;
  payload["position"] = position;

  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendDiag(const char* level, const String& message) {
  StaticJsonDocument<320> doc;
  doc["type"] = "diag.log";
  JsonObject payload = doc.createNestedObject("payload");
  payload["level"] = level;
  payload["message"] = message;

  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::processLine(const String& line) {
  if (line.length() == 0) {
    return;
  }

  DynamicJsonDocument doc(VP_MAX_PACKET_SIZE * 2);
  const DeserializationError err = deserializeJson(doc, line);
  if (err) {
    return;
  }

  const char* type = doc["type"] | "";
  const char* requestId = doc["requestId"] | nullptr;

  if (type == nullptr || type[0] == '\0') {
    return;
  }

  JsonVariantConst payload = doc["payload"];

  if (strcmp(type, "hello") == 0) {
    handleHello(requestId);
    return;
  }

  if (strcmp(type, "settings.apply") == 0) {
    handleSettingsApply(payload, requestId);
    return;
  }

  if (strcmp(type, "display.render") == 0) {
    handleDisplayRender(payload, requestId);
    return;
  }

  if (strcmp(type, "leds.meter") == 0) {
    handleLedsMeter(payload, requestId);
    return;
  }

  if (strcmp(type, "debug.state.get") == 0) {
    handleDebugStateGet(requestId);
    return;
  }

  if (strcmp(type, "debug.strain.calibrate") == 0) {
    handleDebugCalibrateStrain(requestId);
    return;
  }

  if (strcmp(type, "debug.tuning.apply") == 0) {
    handleDebugTuningApply(payload, requestId);
    return;
  }

  if (strcmp(type, "debug.stream.set") == 0) {
    handleDebugStreamSet(payload, requestId);
    return;
  }

  if (strcmp(type, "display.beginFrame") == 0 ||
      strcmp(type, "display.rect") == 0 ||
      strcmp(type, "display.endFrame") == 0 ||
      strcmp(type, "display.fullFrame") == 0) {
    sendNack(requestId, type, "unsupported-type");
    return;
  }

  if (strcmp(type, "device.ping") == 0) {
    sendAck(requestId, type);
    return;
  }

  sendNack(requestId, type, "unsupported-type");
}

void ProtocolServer::handleHello(const char* requestId) {
  sendAck(requestId, "hello");
}

void ProtocolServer::handleSettingsApply(JsonVariantConst payload, const char* requestId) {
  if (payload.is<JsonObjectConst>()) {
    JsonObjectConst obj = payload.as<JsonObjectConst>();
    settings_.detentCount = obj["detentCount"] | settings_.detentCount;
    settings_.detentStrength = obj["detentStrength"] | settings_.detentStrength;
    settings_.snapStrength = obj["snapStrength"] | settings_.snapStrength;
    settings_.ledBrightness = obj["ledBrightness"] | settings_.ledBrightness;
    settings_.displayBrightness = obj["displayBrightness"] | settings_.displayBrightness;
    settings_.encoderInvert = obj["encoderInvert"] | settings_.encoderInvert;
    settings_.buttonLongPressMs = obj["buttonLongPressMs"] | settings_.buttonLongPressMs;
  }

  settings_.clamp();
  haptics_.applySettings(settings_);
  leds_.onSettingsChanged(settings_);
  display_.onSettingsChanged(settings_);

  sendSettingsAck(requestId);
}

void ProtocolServer::handleDisplayRender(JsonVariantConst payload, const char* requestId) {
  display_.applyRenderPayload(payload);
  sendAck(requestId, "display.render");
}

void ProtocolServer::handleLedsMeter(JsonVariantConst payload, const char* requestId) {
  MeterModel meter;
  if (payload.is<JsonObjectConst>()) {
    JsonObjectConst obj = payload.as<JsonObjectConst>();
    meter.rms = clampValue(obj["rms"] | meter.rms, 0.0f, 1.0f);
    meter.peak = clampValue(obj["peak"] | meter.peak, 0.0f, 1.0f);
    meter.muted = obj["muted"] | meter.muted;
    meter.theme = obj["theme"] | meter.theme;
    meter.brightness = clampValue(obj["brightness"] | meter.brightness, 0.0f, 1.0f);
  }

  leds_.applyMeter(meter);
  sendAck(requestId, "leds.meter");
}

void ProtocolServer::handleDebugStateGet(const char* requestId) {
  sendDebugState("request");
  sendAck(requestId, "debug.state.get");
}

void ProtocolServer::handleDebugCalibrateStrain(const char* requestId) {
  strain_.calibrateBaseline();
  sendDebugState("strain-calibrate");
  sendAck(requestId, "debug.strain.calibrate");
}

void ProtocolServer::handleDebugTuningApply(JsonVariantConst payload, const char* requestId) {
  auto strainState = strain_.getDebugState();
  auto hapticsState = haptics_.getDebugState();

  float pressThreshold = strainState.pressThreshold;
  float releaseHysteresis = strainState.releaseHysteresis;
  float forceScale = strainState.forceScale;
  float baselineAlpha = strainState.baselineAlpha;

  float detentStrengthMax = hapticsState.detentStrengthMaxVPerRad;
  float snapStrengthMax = hapticsState.snapStrengthMaxVPerRad;
  float clickPulseVoltage = hapticsState.clickPulseVoltage;
  uint32_t clickPulseMs = hapticsState.clickPulseMs;
  float endstopMinPos = hapticsState.endstopMinPos;
  float endstopMaxPos = hapticsState.endstopMaxPos;
  float endstopMinStrength = hapticsState.endstopMinStrength;
  float endstopMaxStrength = hapticsState.endstopMaxStrength;

  if (payload.is<JsonObjectConst>()) {
    JsonObjectConst obj = payload.as<JsonObjectConst>();
    pressThreshold = obj["pressThreshold"] | pressThreshold;
    releaseHysteresis = obj["releaseHysteresis"] | releaseHysteresis;
    forceScale = obj["forceScale"] | forceScale;
    baselineAlpha = obj["baselineAlpha"] | baselineAlpha;

    detentStrengthMax = obj["detentStrengthMaxVPerRad"] | detentStrengthMax;
    snapStrengthMax = obj["snapStrengthMaxVPerRad"] | snapStrengthMax;
    clickPulseVoltage = obj["clickPulseVoltage"] | clickPulseVoltage;
    clickPulseMs = obj["clickPulseMs"] | clickPulseMs;
    endstopMinPos = obj["endstopMinPos"] | endstopMinPos;
    endstopMaxPos = obj["endstopMaxPos"] | endstopMaxPos;
    endstopMinStrength = obj["endstopMinStrength"] | endstopMinStrength;
    endstopMaxStrength = obj["endstopMaxStrength"] | endstopMaxStrength;
  }

  strain_.applyDebugTuning(pressThreshold, releaseHysteresis, forceScale, baselineAlpha);
  haptics_.applyDebugTuning(
      detentStrengthMax,
      snapStrengthMax,
      clickPulseVoltage,
      clickPulseMs,
      endstopMinPos,
      endstopMaxPos,
      endstopMinStrength,
      endstopMaxStrength);

  sendDebugState("tuning-apply");
  sendAck(requestId, "debug.tuning.apply");
}

void ProtocolServer::handleDebugStreamSet(JsonVariantConst payload, const char* requestId) {
  if (payload.is<JsonObjectConst>()) {
    JsonObjectConst obj = payload.as<JsonObjectConst>();
    debugStreamEnabled_ = obj["enabled"] | debugStreamEnabled_;
    debugStreamIntervalMs_ = clampValue(static_cast<uint32_t>(obj["intervalMs"] | debugStreamIntervalMs_), 50U, 2000U);
  }

  sendAck(requestId, "debug.stream.set");
}

void ProtocolServer::sendHello() {
  StaticJsonDocument<256> doc;
  doc["type"] = "hello";
  JsonObject payload = doc.createNestedObject("payload");
  payload["deviceId"] = config_.identity.deviceId;
  payload["firmwareVersion"] = config_.identity.firmwareVersion;

  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendCapabilities() {
  StaticJsonDocument<320> doc;
  doc["type"] = "capabilities";
  JsonObject payload = doc.createNestedObject("payload");
  payload["protocolVersion"] = config_.identity.protocolVersion;
  payload["firmwareVersion"] = config_.identity.firmwareVersion;
  payload["supportsDisplay"] = config_.identity.supportsDisplay;
  payload["supportsLedMeter"] = config_.identity.supportsLedMeter;
  payload["maxPacketSize"] = config_.identity.maxPacketSize;
  payload["maxFrameRateHz"] = config_.identity.maxFrameRateHz;

  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendAck(const char* requestId, const char* forType) {
  StaticJsonDocument<256> doc;
  doc["type"] = "ack";
  if (requestId != nullptr && requestId[0] != '\0') {
    doc["requestId"] = requestId;
  }

  JsonObject payload = doc.createNestedObject("payload");
  payload["ok"] = true;
  payload["type"] = forType;

  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendSettingsAck(const char* requestId) {
  StaticJsonDocument<512> doc;
  doc["type"] = "ack";
  if (requestId != nullptr && requestId[0] != '\0') {
    doc["requestId"] = requestId;
  }

  JsonObject payload = doc.createNestedObject("payload");
  payload["ok"] = true;
  payload["type"] = "settings.apply";

  JsonObject effective = payload.createNestedObject("effective");
  effective["detentCount"] = settings_.detentCount;
  effective["detentStrength"] = settings_.detentStrength;
  effective["snapStrength"] = settings_.snapStrength;
  effective["ledBrightness"] = settings_.ledBrightness;
  effective["displayBrightness"] = settings_.displayBrightness;
  effective["encoderInvert"] = settings_.encoderInvert;
  effective["buttonLongPressMs"] = settings_.buttonLongPressMs;

  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendNack(const char* requestId, const char* forType, const char* reason) {
  StaticJsonDocument<256> doc;
  doc["type"] = "nack";
  if (requestId != nullptr && requestId[0] != '\0') {
    doc["requestId"] = requestId;
  }

  JsonObject payload = doc.createNestedObject("payload");
  payload["ok"] = false;
  payload["type"] = forType;
  payload["reason"] = reason;

  serializeJson(doc, Serial);
  Serial.print('\n');
}

void ProtocolServer::sendDebugState(const char* source) {
  const StrainDebugState strain = strain_.getDebugState();
  const HapticsDebugState haptics = haptics_.getDebugState();

  StaticJsonDocument<768> doc;
  doc["type"] = "debug.state";
  JsonObject payload = doc.createNestedObject("payload");
  payload["deviceId"] = config_.identity.deviceId;
  payload["source"] = source;
  payload["uptimeMs"] = millis();
  payload["detentCount"] = settings_.detentCount;
  payload["detentStrength"] = settings_.detentStrength;
  payload["snapStrength"] = settings_.snapStrength;

  JsonObject strainObj = payload.createNestedObject("strain");
  strainObj["ready"] = strain.ready;
  strainObj["pressed"] = strain.pressed;
  strainObj["force"] = strain.force;
  strainObj["baseline"] = strain.baseline;
  strainObj["filtered"] = strain.filtered;
  strainObj["pressThreshold"] = strain.pressThreshold;
  strainObj["releaseHysteresis"] = strain.releaseHysteresis;
  strainObj["forceScale"] = strain.forceScale;
  strainObj["baselineAlpha"] = strain.baselineAlpha;

  JsonObject hapticsObj = payload.createNestedObject("haptics");
  hapticsObj["ready"] = haptics.ready;
  hapticsObj["position"] = haptics.position;
  hapticsObj["detentStrengthMaxVPerRad"] = haptics.detentStrengthMaxVPerRad;
  hapticsObj["snapStrengthMaxVPerRad"] = haptics.snapStrengthMaxVPerRad;
  hapticsObj["clickPulseVoltage"] = haptics.clickPulseVoltage;
  hapticsObj["clickPulseMs"] = haptics.clickPulseMs;
  hapticsObj["endstopMinPos"] = haptics.endstopMinPos;
  hapticsObj["endstopMaxPos"] = haptics.endstopMaxPos;
  hapticsObj["endstopMinStrength"] = haptics.endstopMinStrength;
  hapticsObj["endstopMaxStrength"] = haptics.endstopMaxStrength;

  serializeJson(doc, Serial);
  Serial.print('\n');
}

}  // namespace vp