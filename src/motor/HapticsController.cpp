#include "motor/HapticsController.h"

#include <cmath>

#include "util/MathUtil.h"

namespace vp {

bool HapticsController::begin(const DeviceConfig& config, Sensor* sensor) {
  ready_ = false;

  if (sensor == nullptr) {
    return false;
  }

  config_ = config;
  settings_.clamp();

  motor_.reset(new BLDCMotor(config_.hardware.motorPolePairs));
  driver_.reset(new BLDCDriver6PWM(
      config_.pins.uh,
      config_.pins.ul,
      config_.pins.vh,
      config_.pins.vl,
      config_.pins.wh,
      config_.pins.wl));

  pinMode(config_.pins.diag, INPUT_PULLUP);

  driver_->voltage_power_supply = config_.hardware.motorSupplyVoltage;
  driver_->voltage_limit = config_.hardware.motorVoltageLimit;
  driver_->pwm_frequency = config_.hardware.motorPwmFrequency;
  const int driverInitStatus = driver_->init();
  if (driverInitStatus == 0 || !driver_->initialized) {
    return false;
  }

  motor_->linkDriver(driver_.get());
  motor_->linkSensor(sensor);
  motor_->foc_modulation = FOCModulationType::SinePWM;
  motor_->torque_controller = TorqueControlType::voltage;
  motor_->controller = MotionControlType::torque;
  motor_->voltage_limit = config_.hardware.motorVoltageLimit;

  motor_->init();
  if (motor_->motor_status == FOCMotorStatus::motor_init_failed) {
    motor_->disable();
    return false;
  }

  const int focInitStatus = motor_->initFOC();
  if (focInitStatus == 0 || motor_->motor_status != FOCMotorStatus::motor_ready) {
    motor_->disable();
    return false;
  }

  const float angle = motor_->shaft_angle;
  if (!std::isfinite(angle)) {
    motor_->disable();
    return false;
  }

  const float step = kTwoPi / static_cast<float>(settings_.detentCount);
  detentOffset_ = angle;
  lastDetentIndex_ = static_cast<int>(roundf((angle - detentOffset_) / step));
  currentDetentIndex_ = lastDetentIndex_;

  ready_ = true;
  return true;
}

void HapticsController::applySettings(const DeviceRuntimeSettings& settings) {
  settings_ = settings;
  settings_.clamp();
}

void HapticsController::tick(uint32_t nowMs) {
  if (!ready_) {
    return;
  }

  motor_->loopFOC();

  const float angle = motor_->shaft_angle;
  const float step = kTwoPi / static_cast<float>(settings_.detentCount);
  const float rel = angle - detentOffset_;
  currentDetentIndex_ = static_cast<int>(roundf(rel / step));

  const int rawDelta = currentDetentIndex_ - lastDetentIndex_;
  if (rawDelta != 0) {
    const int direction = settings_.encoderInvert ? -1 : 1;
    pendingDelta_ += rawDelta * direction;
    lastDetentIndex_ = currentDetentIndex_;
  }

  const float detentAngle = static_cast<float>(currentDetentIndex_) * step + detentOffset_;
  const float detentErr = shortestAngleError(detentAngle, angle);

  float uq = 0.0f;
  uq += computeDetentTorque(angle);
  uq += computeSnapTorque(detentErr);
  uq += computeEndstopTorque(angle);
  uq += computeClickTorque(nowMs);

  uq = clampValue(uq, -config_.hardware.endstopMaxVoltage, config_.hardware.endstopMaxVoltage);
  motor_->move(uq);
}

void HapticsController::triggerClickPulse(uint32_t nowMs) {
  clickActive_ = true;
  clickStartMs_ = nowMs;
}

int HapticsController::consumeDetentDelta() {
  const int delta = pendingDelta_;
  pendingDelta_ = 0;
  return delta;
}

int HapticsController::currentPosition() const {
  return currentDetentIndex_;
}

bool HapticsController::ready() const {
  return ready_;
}

void HapticsController::applyDebugTuning(float detentStrengthMaxVPerRad,
                                         float snapStrengthMaxVPerRad,
                                         float clickPulseVoltage,
                                         uint32_t clickPulseMs,
                                         float endstopMinPos,
                                         float endstopMaxPos,
                                         float endstopMinStrength,
                                         float endstopMaxStrength) {
  config_.hardware.detentStrengthMaxVPerRad = clampValue(detentStrengthMaxVPerRad, 0.0f, 12.0f);
  config_.hardware.snapStrengthMaxVPerRad = clampValue(snapStrengthMaxVPerRad, 0.0f, 12.0f);
  config_.hardware.clickPulseVoltage = clampValue(clickPulseVoltage, 0.0f, config_.hardware.endstopMaxVoltage);
  config_.hardware.clickPulseMs = clampValue(clickPulseMs, 1U, 1000U);

  if (endstopMinPos < endstopMaxPos) {
    config_.hardware.endstopMinPos = endstopMinPos;
    config_.hardware.endstopMaxPos = endstopMaxPos;
  }

  config_.hardware.endstopMinStrength = clampValue(endstopMinStrength, 0.0f, 20.0f);
  config_.hardware.endstopMaxStrength = clampValue(endstopMaxStrength, 0.0f, 20.0f);
}

HapticsDebugState HapticsController::getDebugState() const {
  HapticsDebugState state;
  state.ready = ready_;
  state.position = currentDetentIndex_;
  state.detentStrengthMaxVPerRad = config_.hardware.detentStrengthMaxVPerRad;
  state.snapStrengthMaxVPerRad = config_.hardware.snapStrengthMaxVPerRad;
  state.clickPulseVoltage = config_.hardware.clickPulseVoltage;
  state.clickPulseMs = config_.hardware.clickPulseMs;
  state.endstopMinPos = config_.hardware.endstopMinPos;
  state.endstopMaxPos = config_.hardware.endstopMaxPos;
  state.endstopMinStrength = config_.hardware.endstopMinStrength;
  state.endstopMaxStrength = config_.hardware.endstopMaxStrength;
  return state;
}

float HapticsController::computeDetentTorque(float angle) const {
  const float step = kTwoPi / static_cast<float>(settings_.detentCount);
  const float rel = angle - detentOffset_;
  const float nearestIndex = roundf(rel / step);
  const float detentAngle = nearestIndex * step + detentOffset_;
  const float err = shortestAngleError(detentAngle, angle);

  const float detentGain = settings_.detentStrength * config_.hardware.detentStrengthMaxVPerRad;
  return detentGain * err;
}

float HapticsController::computeSnapTorque(float detentError) const {
  const float window = config_.hardware.snapWindowRad;
  const float absErr = fabsf(detentError);
  if (absErr >= window || window <= 0.0001f) {
    return 0.0f;
  }

  const float normalized = 1.0f - (absErr / window);
  const float snapGain = settings_.snapStrength * config_.hardware.snapStrengthMaxVPerRad;
  return snapGain * detentError * normalized;
}

float HapticsController::computeEndstopTorque(float angle) const {
  float uq = 0.0f;

  if (angle < config_.hardware.endstopMinPos) {
    const float penetration = config_.hardware.endstopMinPos - angle;
    uq = config_.hardware.endstopMinStrength * penetration;
  } else if (angle > config_.hardware.endstopMaxPos) {
    const float penetration = angle - config_.hardware.endstopMaxPos;
    uq = -config_.hardware.endstopMaxStrength * penetration;
  }

  return clampValue(uq, -config_.hardware.endstopMaxVoltage, config_.hardware.endstopMaxVoltage);
}

float HapticsController::computeClickTorque(uint32_t nowMs) {
  if (!clickActive_) {
    return 0.0f;
  }

  const uint32_t elapsed = nowMs - clickStartMs_;
  const uint32_t duration = config_.hardware.clickPulseMs;
  if (elapsed >= duration || duration == 0U) {
    clickActive_ = false;
    return 0.0f;
  }

  const uint32_t half = duration / 2U;
  if (elapsed < half) {
    return config_.hardware.clickPulseVoltage;
  }

  return -config_.hardware.clickPulseVoltage;
}

}  // namespace vp
