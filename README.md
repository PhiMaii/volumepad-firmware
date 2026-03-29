# VolumePad Firmware (ESP32-S3)

PlatformIO + Arduino firmware for VolumePad hardware.

## Build Targets

- `mt6701`: MT6701 magnetic sensor over SPI
- `as5600`: AS5600 magnetic sensor over I2C (test mode)

## Build

```powershell
platformio run -e mt6701
platformio run -e as5600
```

## Current Features

- USB CDC line-delimited JSON protocol (`hello`, `capabilities`, `settings.apply`, `input.event`, `ack`, `nack`, `diag.log`)
- Haptic motor controller with:
  - virtual detents
  - virtual endstops
  - strain-triggered click pulse
- 3 keys mapped to `button-1..3`
- Strain push mapped to `encoder-main` press
- Shared LED data-line controller (3 key LEDs + 27 ring LEDs)
- ST7789 display scaffold (76x284) with top-level render hooks

## Notes

- Pinout and hardware constants are controlled via `build_flags` in `platformio.ini`.
- Framebuffer display packet types currently return `nack` with `unsupported-type`.
- `display.render` is accepted and routed into scaffolded display logic.