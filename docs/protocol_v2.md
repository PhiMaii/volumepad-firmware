# VolumePad Link Protocol v2 (MVP)

This document defines a clean v2 protocol for the current MVP scope.
It is intentionally **not** backward-compatible.

## 1. Scope

v2 covers:

- master volume control and mute
- low-latency master meter transport to the 27-LED ring
- ring animation and direct LED control from backend
- fixed hardware input behavior
- device settings management
- debug state/tuning and debug stream
- device connection control and health reporting
- service control for restarting audio backend

v2 excludes any user-configurable input mapping layer.

## 2. Transports

### 2.1 UI <-> Agent
- Transport: local IPC (named pipe)
- Encoding: UTF-8 JSON
- Framing: one JSON message per frame

### 2.2 Agent <-> Device
- Transport: USB CDC serial
- Encoding: UTF-8 JSON
- Framing: newline-delimited JSON (one object per line)

## 3. Common Message Envelope

All request/response/event messages use:

```json
{
  "v": 2,
  "type": "request",
  "id": "3c7cc08a8f9940f0a90af7ef311af726",
  "name": "device.connect",
  "tsUtc": "2026-04-01T18:00:00.0000000Z",
  "payload": {}
}
```

Rules:

- `v` is required and must be `2`.
- `type` is `request`, `response`, or `event`.
- `id` is required for `request` and `response`.
- `name` is required and names the method/event.
- `payload` is required (use `{}` if empty).

Response shape:

```json
{
  "v": 2,
  "type": "response",
  "id": "3c7cc08a8f9940f0a90af7ef311af726",
  "name": "device.connect",
  "ok": true,
  "payload": {}
}
```

Error response:

```json
{
  "v": 2,
  "type": "response",
  "id": "3c7cc08a8f9940f0a90af7ef311af726",
  "name": "device.connect",
  "ok": false,
  "error": {
    "code": "not_connected",
    "message": "No device available on selected port."
  },
  "payload": {}
}
```

## 4. Connection Health Model

`connectionState` is one of:

- `disconnected`
- `connecting`
- `connected`
- `reconnecting`
- `error`

Agent publishes health events whenever this state changes.
Tray icon state must map directly to these values.

## 5. UI <-> Agent Methods

## 5.1 Device Connection

- `device.listPorts`
- `device.connect`
- `device.disconnect`
- `device.reconnect`
- `device.getStatus`

`device.listPorts` response payload:

```json
{
  "ports": [
    { "portName": "COM3", "isBusy": false },
    { "portName": "COM7", "isBusy": true }
  ]
}
```

`device.getStatus` response payload:

```json
{
  "connectionState": "connected",
  "portName": "COM3",
  "deviceId": "volumepad-001",
  "firmwareVersion": "2.0.0",
  "lastSeenUtc": "2026-04-01T18:00:00.0000000Z"
}
```

## 5.2 Master Audio Control

- `audio.master.get`
- `audio.master.setVolume`
- `audio.master.setMute`
- `audio.master.toggleMute`

`audio.master.get` response payload:

```json
{
  "volume": 0.64,
  "muted": false,
  "peak": 0.73,
  "rms": 0.42,
  "capturedAtUtc": "2026-04-01T18:00:00.0000000Z"
}
```

## 5.3 Settings

- `settings.get`
- `settings.update`

`settings.update` sends the full settings object (section 7).
Settings are saved only when the user confirms in UI.
`settings.update` is atomic: apply to runtime and persist together.
`settings.update` response must include normalized/effective values so UI reflects what firmware actually uses.

`settings.update` response payload:

```json
{
  "effective": {
    "autoReconnectOnError": true,
    "autoConnectOnStartup": false,
    "volumeStepSize": 0.02,
    "detentCount": 24,
    "detentStrength": 0.65,
    "snapStrength": 0.40,
    "encoderInvert": false,
    "ledBrightness": 0.80,
    "meterMode": "ring_fill",
    "meterColor": "#00D26A",
    "meterBrightness": 0.80,
    "meterSmoothing": 0.25,
    "meterPeakHoldMs": 500,
    "meterMuteRedDurationMs": 700,
    "lowEndstopEnabled": true,
    "lowEndstopPosition": -1.00,
    "lowEndstopStrength": 0.70,
    "highEndstopEnabled": true,
    "highEndstopPosition": 1.00,
    "highEndstopStrength": 0.70
  }
}
```

## 5.4 Debug

- `debug.getState`
- `debug.applyTuning`
- `debug.setStream`

Debug tuning is immediate: no confirm/save flow is required.
After successful debug tuning updates, agent should immediately return/publish updated debug state.
Debug values are device-owned: device is the source of truth.
Agent forwards debug writes to device and uses device-reported state for readback.

## 5.5 Service Control

- `service.restartAudioBackend`

## 5.6 UI Events Published by Agent

- `event.connection.stateChanged`
- `event.audio.masterChanged`
- `event.audio.meterTick`
- `event.settings.applied`
- `event.debug.state`
- `event.diagnostics`

## 6. Fixed Input Behavior (Agent Runtime Rule)

Hardware inputs are interpreted as:

- `button-1` press: toggle master mute
- `button-2` press: no action (inactive)
- `button-3` press: no action (inactive)
- `encoder` rotate clockwise: increase master volume by `volumeStepSize * deltaSteps`
- `encoder` rotate counter-clockwise: decrease master volume by `volumeStepSize * abs(deltaSteps)`
- `encoder` press: no action (inactive)

This behavior is fixed in v2 and not user-remappable.

## 7. Settings Schema (Normal Settings)

Settings payload:

```json
{
  "autoReconnectOnError": true,
  "autoConnectOnStartup": false,
  "volumeStepSize": 0.02,
  "detentCount": 24,
  "detentStrength": 0.65,
  "snapStrength": 0.40,
  "encoderInvert": false,
  "ledBrightness": 0.80,
  "meterMode": "ring_fill",
  "meterColor": "#00D26A",
  "meterBrightness": 0.80,
  "meterSmoothing": 0.25,
  "meterPeakHoldMs": 500,
  "meterMuteRedDurationMs": 700,
  "lowEndstopEnabled": true,
  "lowEndstopPosition": -1.00,
  "lowEndstopStrength": 0.70,
  "highEndstopEnabled": true,
  "highEndstopPosition": 1.00,
  "highEndstopStrength": 0.70
}
```

Validation:

- `autoReconnectOnError`: boolean
- `autoConnectOnStartup`: boolean
- `volumeStepSize`: `0.001..0.20`
- `detentCount`: `0..128`
- `detentStrength`: `0.0..1.0`
- `snapStrength`: `0.0..1.0`
- `ledBrightness`: `0.0..1.0`
- `meterMode`: `ring_fill | vu_peak_hold | peak_indicator`
- `meterColor`: `#RRGGBB`
- `meterBrightness`: `0.0..1.0`
- `meterSmoothing`: `0.0..1.0`
- `meterPeakHoldMs`: `0..3000`
- `meterMuteRedDurationMs`: `50..3000`
- `lowEndstopPosition`, `highEndstopPosition`: normalized knob domain `-1.0..1.0`
- `lowEndstopStrength`, `highEndstopStrength`: `0.0..1.0`

Cross-field rules:

- `lowEndstopPosition < highEndstopPosition`
- if an endstop is disabled, its position/strength are stored but not enforced by firmware

Apply routing:

- `autoReconnectOnError` and `autoConnectOnStartup` are agent-level connection settings
- remaining settings fields are device-facing and are forwarded in `device.applySettings`

## 8. Ring Rendering Contract

Goals:

- low-latency first
- target meter push rate: `50 Hz` (every 20 ms)
- latest frame wins under load (drop stale frames)

Meter mode:

- exactly one meter mode active (`ring_fill`, `vu_peak_hold`, or `peak_indicator`)

Render owner priority (highest to lowest):

1. `mute_override`
2. `animation_stream`
3. `meter`

Rules:

- `animation_stream` fully overwrites meter rendering while active.
- Animation streaming is not limited to startup and may be used for transient flows (for example startup, settings applied, reconnect, diagnostics).
- Frame-by-frame animation streaming is supported.
- Backend may also send direct individual LED updates for targeted effects.

Muted behavior:

- on mute transition to `true`, `mute_override` must interrupt any active animation immediately
- full red override lasts `meterMuteRedDurationMs`
- after override duration, render owner falls back to active `animation_stream` (if any), otherwise `meter`

Global meter visual settings (shared by all meter modes):

- `meterColor`
- `meterBrightness`
- `meterSmoothing`

## 9. Agent <-> Device Methods

## 9.1 Handshake and Capabilities

Device event on connect:

- `device.hello`

Payload:

```json
{
  "deviceId": "volumepad-001",
  "firmwareVersion": "2.0.0",
  "ringLedCount": 27,
  "buttonLedCount": 3
}
```

Agent request:

- `device.applySettings` (section 7 settings payload)

## 9.2 Input Events (Device -> Agent)

- `device.input.button`
- `device.input.encoder`

Button payload:

```json
{
  "buttonId": 1,
  "action": "press",
  "tsUtc": "2026-04-01T18:00:00.0000000Z"
}
```

Encoder payload:

```json
{
  "deltaSteps": 1,
  "pressed": false,
  "tsUtc": "2026-04-01T18:00:00.0000000Z"
}
```

## 9.3 Meter Frames (Agent -> Device)

- `device.meter.frame`

Payload:

```json
{
  "seq": 124551,
  "peak": 0.73,
  "rms": 0.42,
  "muted": false,
  "mode": "ring_fill",
  "color": "#00D26A",
  "brightness": 0.80,
  "smoothing": 0.25,
  "peakHoldMs": 500,
  "muteRedDurationMs": 700
}
```

Rules:

- frames are idempotent latest-state updates
- firmware should drop any frame with `seq` older than last applied meter frame
- frames may continue arriving during animations, but meter rendering is hidden while `animation_stream` owns the ring

## 9.4 Direct Ring LED Command (Agent -> Device)

- `device.ring.setLed`

Payload:

```json
{
  "index": 0,
  "color": "#00D26A",
  "brightness": 0.80
}
```

Rules:

- `index` range is `0..26`
- intended for targeted direct updates
- does not start animation mode by itself

## 9.5 Ring Animation Streaming (Agent -> Device)

- `device.ring.stream.begin`
- `device.ring.stream.frame`
- `device.ring.stream.end`

`device.ring.stream.begin` payload:

```json
{
  "streamId": "settings-apply-20260401-180001",
  "reason": "settings_applied",
  "ledCount": 27
}
```

`device.ring.stream.frame` payload:

```json
{
  "streamId": "settings-apply-20260401-180001",
  "seq": 1,
  "leds": [
    { "index": 0, "color": "#00D26A" },
    { "index": 1, "color": "#00D26A" }
  ],
  "brightness": 0.80
}
```

`device.ring.stream.end` payload:

```json
{
  "streamId": "settings-apply-20260401-180001"
}
```

Rules:

- frame-by-frame streaming is supported and expected for animations
- while stream is active, it fully overwrites meter rendering
- firmware should drop any stream frame with `seq` older than last applied stream frame for the same `streamId`
- if a new `stream.begin` arrives while another is active, newest stream becomes active owner

## 9.6 Ring Mute Override (Agent -> Device)

- `device.ring.muteOverride`

Payload:

```json
{
  "color": "#FF0000",
  "durationMs": 700
}
```

Rules:

- this is highest priority ring owner
- must interrupt active stream/meter immediately

## 9.7 Button Backlight Feedback (Agent -> Device)

- `device.buttonLeds.set`

Payload:

```json
{
  "button1": { "color": "#00D26A", "brightness": 0.60 },
  "button2": { "color": "#202020", "brightness": 0.20 },
  "button3": { "color": "#202020", "brightness": 0.20 }
}
```

## 9.8 Debug Commands

- `debug.getState`
- `debug.applyTuning`
- `debug.setStream`
- `debug.state` (event from device)

`debug.applyTuning` payload:

```json
{
  "detentStrengthMaxVPerRad": 2.0,
  "snapStrengthMaxVPerRad": 2.0,
  "clickPulseVoltage": 1.2,
  "clickPulseMs": 34,
  "endstopMinPos": -1.0,
  "endstopMaxPos": 1.0,
  "endstopMinStrength": 0.7,
  "endstopMaxStrength": 0.7
}
```

`debug.setStream` payload:

```json
{
  "enabled": true,
  "intervalMs": 150
}
```

`debug.state` payload (read-only telemetry fields):

```json
{
  "deviceId": "volumepad-001",
  "source": "firmware",
  "uptimeMs": 912345,
  "hapticsReady": true,
  "position": 42,
  "detentCount": 24,
  "detentStrength": 0.65,
  "snapStrength": 0.40
}
```

Debug ownership rules:

- Debug values are persisted/stored by device firmware.
- `debug.getState` should return current device-owned values (not UI-cached values).
- `debug.applyTuning` forwards requested changes to device; resulting `debug.state` reflects effective values from device.

## 10. Timing, Queues, and Priorities

Priority order:

1. connection/control and ring mute override (`device.connect`, `audio.master.*`, service restarts, `device.ring.muteOverride`)
2. settings and debug commands
3. ring animation stream frames and meter frames

Recommendations:

- process control/settings/debug requests before meter frames
- ring stream and meter queues should be bounded and drop-oldest
- connection-state events must never be blocked by meter traffic

## 11. Error Codes

Standard `error.code` values:

- `unknown_method`
- `invalid_payload`
- `out_of_range`
- `not_connected`
- `device_busy`
- `timeout`
- `internal_error`

## 12. MVP Compliance Checklist

- device connection controls: list/connect/disconnect/reconnect
- connection health state events implemented
- fixed input behavior implemented exactly as section 6
- full settings schema accepted, validated, persisted, and applied only on confirmed `settings.update`
- meter frames pushed at up to 50 Hz with latest-frame-wins behavior
- animation stream begin/frame/end implemented for non-startup and startup transient flows
- direct ring LED command implemented (`device.ring.setLed`)
- mute red feedback duration behavior implemented
- debug get/apply/stream/state implemented with immediate debug-state updates
- service restart command implemented for audio backend
