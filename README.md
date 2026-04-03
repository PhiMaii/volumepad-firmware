# VolumePad Firmware

Firmware for the VolumePad hardware device, built for ESP32-S3 with PlatformIO (Arduino framework).

This repository is a submodule of the main VolumePad umbrella repo and implements device-side behavior:
- input handling (encoder/buttons)
- haptics control
- LED ring rendering
- serial protocol server
- settings persistence

## Role In The Full Stack

In the full VolumePad architecture:
- firmware runs on the hardware device
- the desktop app connects to it over serial
- the protocol contract is defined in the root global docs

## Build And Flash

```powershell
pio run
pio run -t upload
pio device monitor
```

## Project Layout

- `src/`: firmware implementation (`main.cpp` and subsystem modules)
- `include/`: headers for config, protocol, LED, haptics, input, and persistence
- `platformio.ini`: PlatformIO environment and dependencies

## Documentation Policy

Global architecture/protocol/layout docs are maintained only in:
`D:\Daten\Programmieren\volumepad\docs`

Use this submodule docs area only for firmware-specific notes that do not redefine global contracts.
