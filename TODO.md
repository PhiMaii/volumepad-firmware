# Firmware TODO (Debug + App-Control Roadmap)

## Scope
This TODO tracks what still needs to be extended in firmware so the app can fully control/tune the device in production quality.

## Parameter Catalog (App-settable, meaningful)
- detentCount
- detentStrength
- snapStrength
- detentStrengthMaxVPerRad
- snapStrengthMaxVPerRad
- snapWindowRad
- detentOffset
- encoderInvert
- endstopMinPos
- endstopMaxPos
- endstopMinStrength
- endstopMaxStrength
- endstopMaxVoltage
- clickPulseVoltage
- clickPulseMs
- click waveform profile (bipolar/impulse/custom)
- click cooldown / retrigger guard
- strain pressThreshold
- strain releaseHysteresis
- strain forceScale
- strain baselineAlpha
- strain sign invert
- strain filterAlpha
- strain debounceMs
- strain baseline reset/calibration command
- LED brightness (global)
- LED meter mode
- LED meter theme/palette
- LED meter smoothing
- LED ring orientation/reverse
- key LED idle/pressed colors
- display brightness
- debug stream enabled
- debug stream interval
- debug verbosity level
- sensor backend select (MT6701/AS5600)
- MT6701 SPI clock/mode/bit-shift/bit-width
- AS5600 I2C address/bit-width
- motor pwm frequency
- motor voltage limit
- motor supply voltage (if runtime-safe)

## Protocol Extensions Needed (beyond current implementation)
- Add explicit request/response correlation surface for debug commands in agent-facing API (not only ack/nack side effects).
- Define versioned schema for `debug.state` payload with backward-compatible optional fields.
- Add `debug.profile.save/load/list` message set for reusable tuning profiles.
- Add `debug.param.get`/`debug.param.set` generic typed parameter API.
- Add `debug.scope.start/stop` for short burst high-rate telemetry streams.
- Add `debug.log.level` command and structured log categories.

## Firmware Runtime Work Remaining
- Persist selected debug tuning values across reboot (NVS/flash profile store).
- Add safety clamps by parameter class with hardware-safe hard limits.
- Add per-parameter validation error reporting in `nack` payloads.
- Add telemetry timestamps and monotonic sample counters.
- Add stream throttling and drop policy metrics.
- Add dedicated diagnostic counters (parser errors, command rejects, queue drops).
- Add calibration state machine for strain sensor warmup/zeroing.
- Add unit-testable parameter normalization layer (host-side tests).

## Bring-up and Validation
- Add scripted protocol conformance tests for all debug message types.
- Add soak test for continuous `debug.stream.set enabled=true` use.
- Add regression checks for haptic loop stability while debug streaming.
- Validate MT6701 parsing assumptions against real sensor register format and update bit extraction if needed.