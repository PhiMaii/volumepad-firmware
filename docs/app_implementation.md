# App Implementation Guide (AI-First)

This document lays out an ordered implementation plan so an AI agent can build the app in small, testable steps.

Target codebase layout:

- `VolumePadLink.Contracts` solution
- `VolumePadLink.Agent` solution
- `VolumePadLink.UI` solution

## 1. Build Order

Implement in this order to minimize rework:

1. Contracts
2. Agent core
3. Device connection flow
4. Audio master + meter flow
5. Fixed input behavior
6. Ring rendering pipeline
7. Settings flow
8. Debug flow
9. UI screens
10. Tray/background behavior
11. Post-V1 Stream Deck-ready architecture hooks

## 2. Contracts Solution

Create shared DTOs/envelopes first:

- IPC envelope (`v`, `type`, `id`, `name`, `payload`, `ok`, `error`)
- request/response models for:
  - device connection (`listPorts`, `connect`, `disconnect`, `reconnect`, `status`)
  - master audio (`get`, `setVolume`, `setMute`, `toggleMute`)
  - settings (`get`, `update`)
  - debug (`getState`, `applyTuning`, `setStream`)
  - service control (`restartAudioBackend`)
- event models:
  - connection state changed
  - master changed
  - meter tick
  - settings applied
  - debug state
  - diagnostics
- settings models:
  - `autoReconnectOnError`
  - `autoConnectOnStartup`
  - all normal settings fields from `app-summary-minimal.md`

## 3. Agent Solution

### 3.1 Core Host + State

- host process + DI
- central state store (single source of truth for runtime state)
- persistent settings store
- command router with versioned contract handling
- event hub/pub-sub

### 3.2 Device Service

- list COM ports
- connect/disconnect/reconnect
- health states: `connecting`, `connected`, `disconnected`, `reconnecting`, `error`
- auto reconnect upon error toggle
- auto connect upon startup toggle
- serial IO framing and message parsing

### 3.3 Audio Service

- master volume get/set/toggle mute
- read master peak/rms from Windows side
- low-latency meter loop target `50 Hz`

### 3.4 Fixed Input Service

Hardcode behavior:

- button 1 -> toggle master mute
- button 2 -> inactive
- button 3 -> inactive
- encoder turn -> +/- by `volumeStepSize`
- encoder press -> inactive

### 3.5 Ring Rendering Service

Implement owner arbitration:

1. `mute_override`
2. `animation_stream`
3. `meter`

Support:

- meter frames
- direct single LED updates
- frame-by-frame animation stream (`begin/frame/end`)
- mute override immediate interruption

### 3.6 Settings Service

- stage in UI, apply on confirm via `settings.update`
- `settings.update` returns normalized/effective values
- persist applied settings
- forward device-facing settings to device

### 3.7 Debug Service

Debug values are device-owned:

- read state from device
- send tuning updates to device
- publish immediate updated debug state after writes
- debug stream on/off + interval support

### 3.8 Post-V1 Stream Deck Hooks (No full feature)

- isolate master state provider + event publisher so a Stream Deck endpoint can be attached later
- keep endpoint interfaces internal and optional

## 4. UI Solution

Implement screens in this order:

1. Device page
2. Settings editor + confirm/apply flow
3. Debug page (immediate apply)
4. General settings page
5. Tray integration

Required UI behavior:

- COM port list + connect/disconnect/reconnect actions
- connection health visible at all times
- normalized/effective settings reflected after apply
- debug values update immediately after any debug write
- tray icon mirrors connection state

## 5. Suggested Milestones

### Milestone A: Vertical Slice

- connect device
- get/set master volume
- toggle mute
- one basic meter frame flow

### Milestone B: Settings + Persistence

- full settings object
- apply-confirm flow
- effective-values response wired into UI

### Milestone C: Ring Features

- meter mode rendering
- animation stream
- mute override priority

### Milestone D: Debug + Tray

- debug get/apply/stream
- tray controls + state icon

## 6. Testing Suggestions

## 6.1 Unit Tests

- contracts serialization/deserialization
- settings validation and normalization
- input behavior mapping logic
- ring owner arbitration transitions
- connection state transition rules

## 6.2 Integration Tests

- agent + simulated device serial session
- settings apply returns effective values
- reconnect behavior with cable-drop simulation
- meter loop throughput under load (drop-oldest behavior)
- debug roundtrip (apply -> readback from device-owned state)

## 6.3 UI/Smoke Tests

- connect flow from COM selection
- health-state rendering
- settings confirm/apply cycle
- debug immediate update cycle
- tray icon state changes

## 6.4 Recommended Regression Scenarios

- rapid mute/unmute while animation stream is active
- disconnect during settings apply
- reconnect during debug stream
- invalid settings payload fallback handling

## 7. Codebase Tips for AI Agents

- Build contract-first. Do not implement ad-hoc payloads in Agent/UI before DTOs exist.
- Keep one source of truth for runtime state in Agent.
- Make services small and single-responsibility.
- Prefer explicit state machines for connection and ring owner arbitration.
- Keep timing constants centralized.
- Always return machine-readable error codes, not only text messages.
- Add exhaustive logging around serial IO boundaries and state transitions.
- Implement simulation hooks early (serial/device simulator) for fast test loops.
- Avoid hidden cross-service mutations; route mutations through dedicated methods.
- Gate post-V1 Stream Deck logic behind clear feature flags or separate registration paths.

## 8. Definition of Done (V1)

- all fixed controls behave exactly as specified
- connection toggles and health states work end-to-end
- settings apply-confirm flow returns and displays effective values
- debug remains device-owned with immediate UI refresh
- meter runs low-latency with ring arbitration rules
- tray mode works with stateful icon
- tests cover critical control, settings, connection, and debug paths
