# VolumePad Link - Minimal App Scope

Revision note: control behavior is fixed and no user-configurable assignment layer is included.

## Overview
VolumePad Link is a desktop companion app focused on:

- Master volume control (including mute)
- Live master peak/RMS metering
- Hardware device configuration and tuning
- Stream Deck-ready backend architecture (Stream Deck features are post-V1)

## Hardware Lighting Clarification
- The device has an LED ring with `27` LEDs around the knob.
- This ring is used to visualize the master meter.
- The device also has `3` separate button backlight LEDs.
- Each button has one dedicated LED for visual feedback.

## Audio Level
- Audio level should prioritize low latency.
- Meter data is sourced from the current master output level on Windows.
- Target update rate is `50 Hz`.
- Exactly one meter mode is active at a time.
- Supported modes:
  - `Bar / Ring Fill`
  - `VU Meter (with Peak Hold)`
  - `Peak Indicator`
- Meter visualization settings are global (shared across all modes):
  - color
  - brightness
  - smoothing
- Ring animation support:
  - ring animations are not limited to startup
  - animations can be used for transient states (for example startup, applying settings, reconnect feedback)
  - animation data can be streamed frame-by-frame from the backend
  - backend may also send direct individual LED updates when needed
  - while an animation is active, it fully overwrites meter rendering
- Muted feedback behavior:
  - when muted, show explicit visual feedback by turning the meter fully red
  - muted feedback overrides any active animation immediately
  - keep this state for a configurable duration before returning to normal muted behavior

## App Structure

### Device Page
- Provide explicit connection controls:
  - Connect
  - Disconnect
  - Reconnect
- Show a device list of currently available COM ports for manual selection
- Display clear live connection health state to the user:
  - Connecting
  - Connected
  - Disconnected
  - Reconnecting
  - Error
- Provide connection behavior toggles in settings:
  - auto reconnect upon error
  - auto connect upon startup
- Tune device settings like force-feedback behavior parameters
- Tune device brightness
- Configure master volume step size for encoder turns
- Configure low and high endstops:
  - enable/disable low endstop
  - set low endstop position
  - set low endstop strength
  - enable/disable high endstop
  - set high endstop position
  - set high endstop strength
- Device setting edits are staged and only saved and applied when the user confirms.
- After settings are applied, show normalized/effective values returned by backend so UI reflects what firmware actually uses.
- Apply and persist device settings

### Tray and Background Runtime
- Agent runs in the background
- App operates as a tray app
- Tray icon reflects current device connection state
- Tray menu provides quick open/exit, connect/reconnect/disconnect controls

### Debug Section
- Live view of runtime telemetry and tuning values
- Real-time parameter tuning for haptics/endstop behavior
- Debug values are device-owned (device is source of truth)
- Backend reads current debug values from device and forwards debug changes to device
- Debug tuning edits apply immediately (no confirm flow)
- Debug state/readouts should update immediately after each debug change
- Toggle/debug stream controls for continuous monitoring

### General Settings
- Restart Stream Deck endpoint (post-V1)
- Restart audio backend
- Keep app-level preferences persisted across restarts

## Stream Deck (Post-V1)
- Stream Deck features are intentionally not implemented in V1.
- Backend architecture should be laid out so these features can be added later with minimal refactoring.
- Planned Stream Deck capabilities:
  - show current master volume on key
  - show speaker icon and crossed speaker icon when muted
  - toggle master mute on key press
  - increase/decrease master volume by configurable step (`x`)
  - expose device connection health to Stream Deck (`connecting`, `connected`, `disconnected`, `reconnecting`, `error`)

## Normal Settings (High Value)
- `autoReconnectOnError`
- `autoConnectOnStartup`
- `volumeStepSize`
- `detentCount`
- `detentStrength`
- `snapStrength`
- `encoderInvert`
- `ledBrightness`
- `meterMode`
- `meterColor`
- `meterBrightness`
- `meterSmoothing`
- `meterPeakHoldMs`
- `meterMuteRedDurationMs`
- `lowEndstopEnabled`
- `lowEndstopPosition`
- `lowEndstopStrength`
- `highEndstopEnabled`
- `highEndstopPosition`
- `highEndstopStrength`

## Debug Params (MVP)

### Editable Debug Params
- `detentStrengthMaxVPerRad`
- `snapStrengthMaxVPerRad`
- `clickPulseVoltage`
- `clickPulseMs`
- `endstopMinPos`
- `endstopMaxPos`
- `endstopMinStrength`
- `endstopMaxStrength`
- `debugStreamEnabled`
- `debugStreamIntervalMs`

### Read-Only Debug Telemetry
- `deviceId`
- `source`
- `uptimeMs`
- `hapticsReady`
- `position`
- `detentCount`
- `detentStrength`
- `snapStrength`

## Fixed Control Behavior
- Button 1: toggle master mute
- Button 2: inactive
- Button 3: inactive
- Encoder turn clockwise: increase master volume by configured step size
- Encoder turn counter-clockwise: decrease master volume by configured step size
- Encoder press: inactive

## Result
This scope keeps the app compact and predictable with fixed input behavior and focused device-level tuning.
