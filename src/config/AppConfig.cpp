#include "config/AppConfig.h"

namespace vp {

static AngleSensorBackend resolveBackend() {
#if VP_SENSOR_BACKEND_AS5600 || VP_SENSOR_BACKEND_AS6500
  return AngleSensorBackend::As5600I2c;
#else
  return AngleSensorBackend::Mt6701Spi;
#endif
}

const DeviceConfig& getDeviceConfig() {
  static const DeviceConfig config{
      PinConfig{
          VP_PIN_UL,
          VP_PIN_UH,
          VP_PIN_VL,
          VP_PIN_VH,
          VP_PIN_WL,
          VP_PIN_WH,
          VP_PIN_DIAG,
          VP_PIN_I2C_SDA,
          VP_PIN_I2C_SCL,
          VP_PIN_SPI_SCK,
          VP_PIN_SPI_MISO,
          VP_PIN_SPI_MOSI,
          VP_PIN_SENSOR_CS,
          VP_PIN_HX711_DOUT,
          VP_PIN_HX711_SCK,
          VP_PIN_KEY_1,
          VP_PIN_KEY_2,
          VP_PIN_KEY_3,
          VP_PIN_LED_DATA,
          VP_PIN_TFT_CS,
          VP_PIN_TFT_DC,
          VP_PIN_TFT_RST,
          VP_PIN_TFT_BL,
      },
      HardwareConfig{
          VP_MOTOR_POLE_PAIRS,
          VP_MOTOR_SUPPLY_VOLTAGE,
          VP_MOTOR_VOLTAGE_LIMIT,
          VP_MOTOR_PWM_FREQUENCY,
          VP_KEY_LED_COUNT,
          VP_RING_LED_COUNT,
          VP_ENDSTOP_MIN_POS,
          VP_ENDSTOP_MAX_POS,
          VP_ENDSTOP_MIN_STRENGTH,
          VP_ENDSTOP_MAX_STRENGTH,
          VP_ENDSTOP_MAX_VOLTAGE,
          VP_DETENT_STRENGTH_MAX_V_PER_RAD,
          VP_SNAP_STRENGTH_MAX_V_PER_RAD,
          VP_SNAP_WINDOW_RAD,
          VP_STRAIN_PRESS_THRESHOLD,
          VP_STRAIN_RELEASE_HYSTERESIS,
          VP_STRAIN_FORCE_SCALE,
          VP_STRAIN_INVERT_SIGN != 0,
          VP_STRAIN_BASELINE_ALPHA,
          VP_CLICK_PULSE_VOLTAGE,
          VP_CLICK_PULSE_MS,
          VP_MT6701_SPI_HZ,
          static_cast<uint8_t>(VP_MT6701_SPI_MODE),
          static_cast<uint8_t>(VP_MT6701_BITS),
          static_cast<uint8_t>(VP_MT6701_BIT_SHIFT),
          static_cast<uint8_t>(VP_AS5600_I2C_ADDR),
          static_cast<uint8_t>(VP_AS5600_BITS),
      },
      DeviceIdentityConfig{
          VP_DEVICE_ID,
          VP_FIRMWARE_VERSION,
          VP_PROTOCOL_VERSION,
          VP_SUPPORTS_DISPLAY != 0,
          VP_SUPPORTS_LED_METER != 0,
          VP_MAX_PACKET_SIZE,
          VP_MAX_FRAME_RATE_HZ,
      },
      resolveBackend(),
  };

  return config;
}

}  // namespace vp
