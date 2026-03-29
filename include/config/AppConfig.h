#pragma once

#include <Arduino.h>

#ifndef VP_SENSOR_BACKEND_MT6701
#define VP_SENSOR_BACKEND_MT6701 0
#endif

#ifndef VP_SENSOR_BACKEND_AS5600
#define VP_SENSOR_BACKEND_AS5600 0
#endif

#ifndef VP_FIRMWARE_VERSION
#define VP_FIRMWARE_VERSION "fw-0.1.0"
#endif

#ifndef VP_DEVICE_ID
#define VP_DEVICE_ID "volumepad-001"
#endif

#ifndef VP_PROTOCOL_VERSION
#define VP_PROTOCOL_VERSION "1"
#endif

#ifndef VP_SUPPORTS_DISPLAY
#define VP_SUPPORTS_DISPLAY 1
#endif

#ifndef VP_SUPPORTS_LED_METER
#define VP_SUPPORTS_LED_METER 1
#endif

#ifndef VP_MAX_PACKET_SIZE
#define VP_MAX_PACKET_SIZE 1024
#endif

#ifndef VP_MAX_FRAME_RATE_HZ
#define VP_MAX_FRAME_RATE_HZ 60
#endif

#ifndef VP_PIN_UL
#define VP_PIN_UL 35
#endif

#ifndef VP_PIN_UH
#define VP_PIN_UH 36
#endif

#ifndef VP_PIN_VL
#define VP_PIN_VL 37
#endif

#ifndef VP_PIN_VH
#define VP_PIN_VH 38
#endif

#ifndef VP_PIN_WL
#define VP_PIN_WL 39
#endif

#ifndef VP_PIN_WH
#define VP_PIN_WH 40
#endif

#ifndef VP_PIN_DIAG
#define VP_PIN_DIAG 15
#endif

#ifndef VP_PIN_I2C_SDA
#define VP_PIN_I2C_SDA 20
#endif

#ifndef VP_PIN_I2C_SCL
#define VP_PIN_I2C_SCL 21
#endif

#ifndef VP_PIN_SPI_SCK
#define VP_PIN_SPI_SCK 12
#endif

#ifndef VP_PIN_SPI_MISO
#define VP_PIN_SPI_MISO 13
#endif

#ifndef VP_PIN_SPI_MOSI
#define VP_PIN_SPI_MOSI 11
#endif

#ifndef VP_PIN_SENSOR_CS
#define VP_PIN_SENSOR_CS 10
#endif

#ifndef VP_PIN_HX711_DOUT
#define VP_PIN_HX711_DOUT 4
#endif

#ifndef VP_PIN_HX711_SCK
#define VP_PIN_HX711_SCK 5
#endif

#ifndef VP_PIN_KEY_1
#define VP_PIN_KEY_1 6
#endif

#ifndef VP_PIN_KEY_2
#define VP_PIN_KEY_2 7
#endif

#ifndef VP_PIN_KEY_3
#define VP_PIN_KEY_3 8
#endif

#ifndef VP_PIN_LED_DATA
#define VP_PIN_LED_DATA 9
#endif

#ifndef VP_PIN_TFT_CS
#define VP_PIN_TFT_CS 14
#endif

#ifndef VP_PIN_TFT_DC
#define VP_PIN_TFT_DC 16
#endif

#ifndef VP_PIN_TFT_RST
#define VP_PIN_TFT_RST 17
#endif

#ifndef VP_PIN_TFT_BL
#define VP_PIN_TFT_BL 18
#endif

#ifndef VP_MOTOR_POLE_PAIRS
#define VP_MOTOR_POLE_PAIRS 7
#endif

#ifndef VP_MOTOR_SUPPLY_VOLTAGE
#define VP_MOTOR_SUPPLY_VOLTAGE 5.0f
#endif

#ifndef VP_MOTOR_VOLTAGE_LIMIT
#define VP_MOTOR_VOLTAGE_LIMIT 4.0f
#endif

#ifndef VP_MOTOR_PWM_FREQUENCY
#define VP_MOTOR_PWM_FREQUENCY 25000
#endif

#ifndef VP_DETENT_COUNT_DEFAULT
#define VP_DETENT_COUNT_DEFAULT 24
#endif

#ifndef VP_DETENT_STRENGTH_DEFAULT
#define VP_DETENT_STRENGTH_DEFAULT 0.65f
#endif

#ifndef VP_SNAP_STRENGTH_DEFAULT
#define VP_SNAP_STRENGTH_DEFAULT 0.4f
#endif

#ifndef VP_LED_BRIGHTNESS_DEFAULT
#define VP_LED_BRIGHTNESS_DEFAULT 0.8f
#endif

#ifndef VP_DISPLAY_BRIGHTNESS_DEFAULT
#define VP_DISPLAY_BRIGHTNESS_DEFAULT 0.8f
#endif

#ifndef VP_BUTTON_LONG_PRESS_DEFAULT
#define VP_BUTTON_LONG_PRESS_DEFAULT 450
#endif

#ifndef VP_KEY_LED_COUNT
#define VP_KEY_LED_COUNT 3
#endif

#ifndef VP_RING_LED_COUNT
#define VP_RING_LED_COUNT 27
#endif

#ifndef VP_ENDSTOP_MIN_POS
#define VP_ENDSTOP_MIN_POS -1.5f
#endif

#ifndef VP_ENDSTOP_MAX_POS
#define VP_ENDSTOP_MAX_POS 1.5f
#endif

#ifndef VP_ENDSTOP_MIN_STRENGTH
#define VP_ENDSTOP_MIN_STRENGTH 5.0f
#endif

#ifndef VP_ENDSTOP_MAX_STRENGTH
#define VP_ENDSTOP_MAX_STRENGTH 2.0f
#endif

#ifndef VP_ENDSTOP_MAX_VOLTAGE
#define VP_ENDSTOP_MAX_VOLTAGE 5.0f
#endif

#ifndef VP_DETENT_STRENGTH_MAX_V_PER_RAD
#define VP_DETENT_STRENGTH_MAX_V_PER_RAD 2.0f
#endif

#ifndef VP_SNAP_STRENGTH_MAX_V_PER_RAD
#define VP_SNAP_STRENGTH_MAX_V_PER_RAD 2.0f
#endif

#ifndef VP_SNAP_WINDOW_RAD
#define VP_SNAP_WINDOW_RAD 0.25f
#endif

#ifndef VP_STRAIN_PRESS_THRESHOLD
#define VP_STRAIN_PRESS_THRESHOLD 6000.0f
#endif

#ifndef VP_STRAIN_RELEASE_HYSTERESIS
#define VP_STRAIN_RELEASE_HYSTERESIS 2000.0f
#endif

#ifndef VP_STRAIN_FORCE_SCALE
#define VP_STRAIN_FORCE_SCALE 1.0f
#endif

#ifndef VP_STRAIN_INVERT_SIGN
#define VP_STRAIN_INVERT_SIGN 1
#endif

#ifndef VP_STRAIN_BASELINE_ALPHA
#define VP_STRAIN_BASELINE_ALPHA 0.0025f
#endif

#ifndef VP_CLICK_PULSE_VOLTAGE
#define VP_CLICK_PULSE_VOLTAGE 1.2f
#endif

#ifndef VP_CLICK_PULSE_MS
#define VP_CLICK_PULSE_MS 34
#endif

#ifndef VP_MT6701_SPI_HZ
#define VP_MT6701_SPI_HZ 1000000
#endif

#ifndef VP_MT6701_SPI_MODE
#define VP_MT6701_SPI_MODE 0
#endif

#ifndef VP_MT6701_BITS
#define VP_MT6701_BITS 14
#endif

#ifndef VP_MT6701_BIT_SHIFT
#define VP_MT6701_BIT_SHIFT 0
#endif

#ifndef VP_AS5600_I2C_ADDR
#define VP_AS5600_I2C_ADDR 0x36
#endif

#ifndef VP_AS5600_BITS
#define VP_AS5600_BITS 12
#endif

#define VP_LED_TOTAL_COUNT (VP_KEY_LED_COUNT + VP_RING_LED_COUNT)

namespace vp {

enum class AngleSensorBackend {
  Mt6701Spi,
  As5600I2c,
};

struct PinConfig {
  int ul;
  int uh;
  int vl;
  int vh;
  int wl;
  int wh;
  int diag;

  int i2cSda;
  int i2cScl;

  int spiSck;
  int spiMiso;
  int spiMosi;
  int sensorCs;

  int hx711Dout;
  int hx711Sck;

  int key1;
  int key2;
  int key3;

  int ledData;

  int tftCs;
  int tftDc;
  int tftRst;
  int tftBl;
};

struct HardwareConfig {
  int motorPolePairs;
  float motorSupplyVoltage;
  float motorVoltageLimit;
  int motorPwmFrequency;

  int keyLedCount;
  int ringLedCount;

  float endstopMinPos;
  float endstopMaxPos;
  float endstopMinStrength;
  float endstopMaxStrength;
  float endstopMaxVoltage;

  float detentStrengthMaxVPerRad;
  float snapStrengthMaxVPerRad;
  float snapWindowRad;

  float strainPressThreshold;
  float strainReleaseHysteresis;
  float strainForceScale;
  bool strainInvertSign;
  float strainBaselineAlpha;

  float clickPulseVoltage;
  uint32_t clickPulseMs;

  uint32_t mt6701SpiHz;
  uint8_t mt6701SpiMode;
  uint8_t mt6701Bits;
  uint8_t mt6701BitShift;

  uint8_t as5600I2cAddress;
  uint8_t as5600Bits;
};

struct DeviceIdentityConfig {
  const char* deviceId;
  const char* firmwareVersion;
  const char* protocolVersion;
  bool supportsDisplay;
  bool supportsLedMeter;
  int maxPacketSize;
  int maxFrameRateHz;
};

struct DeviceConfig {
  PinConfig pins;
  HardwareConfig hardware;
  DeviceIdentityConfig identity;
  AngleSensorBackend sensorBackend;
};

const DeviceConfig& getDeviceConfig();

}  // namespace vp