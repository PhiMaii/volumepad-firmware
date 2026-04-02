#pragma once

#include <Arduino.h>

namespace vp {

constexpr uint8_t kProtocolVersion = 2;
constexpr char kDeviceId[] = "volumepad-001";
constexpr char kFirmwareVersion[] = "2.0.0";

constexpr uint8_t kButtonLedCount = 3;
constexpr uint8_t kRingLedCount = 27;
constexpr uint8_t kTotalLedCount = kButtonLedCount + kRingLedCount;

constexpr int kPinUl = 35;
constexpr int kPinUh = 36;
constexpr int kPinVl = 37;
constexpr int kPinVh = 38;
constexpr int kPinWl = 39;
constexpr int kPinWh = 40;
constexpr int kPinDiag = 15;
constexpr int kPinI2cSda = 20;
constexpr int kPinI2cScl = 21;
constexpr int kPinButton1 = 4;
constexpr int kPinButton2 = 5;
constexpr int kPinButton3 = 6;
constexpr int kPinLedData = 7;

constexpr int kMotorPolePairs = 7;
constexpr float kMotorSupplyVoltage = 5.0f;
constexpr float kMotorVoltageLimit = 4.0f;
constexpr int kMotorPwmFrequency = 25000;

constexpr float kEndstopRangeRad = 1.5f;
constexpr float kEndstopStrengthMaxVPerRad = 8.0f;

struct PinConfig {
  int ul = kPinUl;
  int uh = kPinUh;
  int vl = kPinVl;
  int vh = kPinVh;
  int wl = kPinWl;
  int wh = kPinWh;
  int diag = kPinDiag;
  int i2cSda = kPinI2cSda;
  int i2cScl = kPinI2cScl;
  int button1 = kPinButton1;
  int button2 = kPinButton2;
  int button3 = kPinButton3;
  int ledData = kPinLedData;
};

struct DeviceConfig {
  PinConfig pins;
  const char* deviceId = kDeviceId;
  const char* firmwareVersion = kFirmwareVersion;
  uint8_t protocolVersion = kProtocolVersion;
};

const DeviceConfig& getDeviceConfig();

}  // namespace vp
