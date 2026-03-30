#include "sensors/AngleSensorManager.h"

#include <cmath>

#include "util/MathUtil.h"

namespace vp {

AngleSensorManager* AngleSensorManager::instance_ = nullptr;

AngleSensorManager::AngleSensorManager()
    : sensor_(AngleSensorManager::readAngleCallback, AngleSensorManager::initCallback) {}

bool AngleSensorManager::begin(const DeviceConfig& config) {
  config_ = config;
  instance_ = this;
  ready_ = false;
  initBus();

  uint16_t raw = 0;
  uint16_t maxRaw = 0;
  bool sampleOk = false;

  // Validate sensor communication before enabling FOC.
  for (int i = 0; i < 16; ++i) {
    if (sampleRawAngle(raw, maxRaw)) {
      sampleOk = true;
      break;
    }
    delay(2);
  }

  if (!sampleOk || maxRaw == 0U) {
    return false;
  }

  cachedAngle_ = (static_cast<float>(raw) / static_cast<float>(maxRaw)) * kTwoPi;
  sensor_.init();

  for (int i = 0; i < 4; ++i) {
    const float angle = readAngleRad();
    if (!std::isfinite(angle)) {
      return false;
    }
    delay(1);
  }

  ready_ = true;
  return true;
}

Sensor* AngleSensorManager::sensor() {
  return &sensor_;
}

bool AngleSensorManager::ready() const {
  return ready_;
}

float AngleSensorManager::lastAngle() const {
  return cachedAngle_;
}

float AngleSensorManager::readAngleCallback() {
  if (instance_ == nullptr) {
    return 0.0f;
  }
  return instance_->readAngleRad();
}

void AngleSensorManager::initCallback() {
  if (instance_ == nullptr) {
    return;
  }
  instance_->initBus();
}

void AngleSensorManager::initBus() {
  if (config_.sensorBackend == AngleSensorBackend::As5600I2c) {
    Wire.begin(config_.pins.i2cSda, config_.pins.i2cScl);
    return;
  }

  pinMode(config_.pins.sensorCs, OUTPUT);
  digitalWrite(config_.pins.sensorCs, HIGH);
  SPI.begin(config_.pins.spiSck, config_.pins.spiMiso, config_.pins.spiMosi, config_.pins.sensorCs);
}

float AngleSensorManager::readAngleRad() {
  uint16_t raw = 0;
  uint16_t maxRaw = 0;
  const bool ok = sampleRawAngle(raw, maxRaw);

  if (!ok || maxRaw == 0) {
    return cachedAngle_;
  }

  cachedAngle_ = (static_cast<float>(raw) / static_cast<float>(maxRaw)) * kTwoPi;
  return cachedAngle_;
}

bool AngleSensorManager::sampleRawAngle(uint16_t& raw, uint16_t& maxRaw) const {
  if (config_.sensorBackend == AngleSensorBackend::As5600I2c) {
    if (config_.hardware.as5600Bits == 0 || config_.hardware.as5600Bits > 15) {
      return false;
    }

    maxRaw = static_cast<uint16_t>((1U << config_.hardware.as5600Bits) - 1U);
    return maxRaw != 0U && readAs5600Raw(raw);
  }

  if (config_.hardware.mt6701Bits == 0 || config_.hardware.mt6701Bits > 15) {
    return false;
  }

  maxRaw = static_cast<uint16_t>((1U << config_.hardware.mt6701Bits) - 1U);
  return maxRaw != 0U && readMt6701Raw(raw);
}

bool AngleSensorManager::readAs5600Raw(uint16_t& raw) const {
  Wire.beginTransmission(config_.hardware.as5600I2cAddress);
  Wire.write(0x0C);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(static_cast<int>(config_.hardware.as5600I2cAddress), 2) != 2) {
    return false;
  }

  const uint16_t high = static_cast<uint16_t>(Wire.read());
  const uint16_t low = static_cast<uint16_t>(Wire.read());
  raw = ((high << 8U) | low) & static_cast<uint16_t>((1U << config_.hardware.as5600Bits) - 1U);
  return true;
}

bool AngleSensorManager::readMt6701Raw(uint16_t& raw) const {
  const uint8_t bits = config_.hardware.mt6701Bits;
  const uint8_t shift = config_.hardware.mt6701BitShift;
  if (bits == 0 || bits > 15) {
    return false;
  }

  SPI.beginTransaction(SPISettings(config_.hardware.mt6701SpiHz, MSBFIRST, config_.hardware.mt6701SpiMode));
  digitalWrite(config_.pins.sensorCs, LOW);
  const uint16_t frame = SPI.transfer16(0x0000);
  digitalWrite(config_.pins.sensorCs, HIGH);
  SPI.endTransaction();

  const uint16_t mask = static_cast<uint16_t>((1U << bits) - 1U);
  raw = static_cast<uint16_t>((frame >> shift) & mask);
  return true;
}

}  // namespace vp
