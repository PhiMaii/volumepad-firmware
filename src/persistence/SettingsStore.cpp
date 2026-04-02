#include "persistence/SettingsStore.h"

namespace vp {

namespace {

constexpr const char* kNamespace = "volumepad";
constexpr const char* kSchemaKey = "schema";

constexpr const char* kAutoReconnectOnError = "s_aroe";
constexpr const char* kAutoConnectOnStartup = "s_acos";
constexpr const char* kVolumeStepSize = "s_vstep";
constexpr const char* kDetentCount = "s_dcnt";
constexpr const char* kDetentStrength = "s_dst";
constexpr const char* kSnapStrength = "s_snp";
constexpr const char* kEncoderInvert = "s_einv";
constexpr const char* kLedBrightness = "s_ledb";
constexpr const char* kMeterMode = "s_mmode";
constexpr const char* kMeterColor = "s_mclr";
constexpr const char* kMeterBrightness = "s_mbr";
constexpr const char* kMeterSmoothing = "s_msm";
constexpr const char* kMeterPeakHoldMs = "s_mph";
constexpr const char* kMeterMuteRedDurationMs = "s_mmr";
constexpr const char* kLowEndstopEnabled = "s_len";
constexpr const char* kLowEndstopPosition = "s_lpos";
constexpr const char* kLowEndstopStrength = "s_lst";
constexpr const char* kHighEndstopEnabled = "s_hen";
constexpr const char* kHighEndstopPosition = "s_hpos";
constexpr const char* kHighEndstopStrength = "s_hst";

constexpr const char* kDetentStrengthMaxVPerRad = "d_dstmx";
constexpr const char* kSnapStrengthMaxVPerRad = "d_ssmx";
constexpr const char* kClickPulseVoltage = "d_cpv";
constexpr const char* kClickPulseMs = "d_cpms";
constexpr const char* kEndstopMinPos = "d_emin";
constexpr const char* kEndstopMaxPos = "d_emax";
constexpr const char* kEndstopMinStrength = "d_esmn";
constexpr const char* kEndstopMaxStrength = "d_esmx";
constexpr const char* kDebugStreamEnabled = "d_sten";
constexpr const char* kDebugStreamIntervalMs = "d_stiv";

}  // namespace

bool SettingsStore::begin() {
  if (started_) {
    return true;
  }

  if (!prefs_.begin(kNamespace, false)) {
    return false;
  }

  const uint8_t schema = prefs_.getUChar(kSchemaKey, 0);
  if (schema != kSchemaVersion) {
    prefs_.clear();
    prefs_.putUChar(kSchemaKey, kSchemaVersion);
  }

  started_ = true;
  return true;
}

void SettingsStore::load(NormalSettings& settings, DebugTuning& tuning) {
  if (!started_ && !begin()) {
    settings.normalize();
    tuning.normalize();
    syncEndstopsFromSettings(settings, tuning);
    return;
  }

  settings.autoReconnectOnError = prefs_.getBool(kAutoReconnectOnError, settings.autoReconnectOnError);
  settings.autoConnectOnStartup = prefs_.getBool(kAutoConnectOnStartup, settings.autoConnectOnStartup);
  settings.volumeStepSize = prefs_.getFloat(kVolumeStepSize, settings.volumeStepSize);
  settings.detentCount = prefs_.getInt(kDetentCount, settings.detentCount);
  settings.detentStrength = prefs_.getFloat(kDetentStrength, settings.detentStrength);
  settings.snapStrength = prefs_.getFloat(kSnapStrength, settings.snapStrength);
  settings.encoderInvert = prefs_.getBool(kEncoderInvert, settings.encoderInvert);
  settings.ledBrightness = prefs_.getFloat(kLedBrightness, settings.ledBrightness);
  settings.meterMode = parseMeterMode(prefs_.getString(kMeterMode, meterModeToString(settings.meterMode)));
  settings.meterColor = prefs_.getString(kMeterColor, settings.meterColor);
  settings.meterBrightness = prefs_.getFloat(kMeterBrightness, settings.meterBrightness);
  settings.meterSmoothing = prefs_.getFloat(kMeterSmoothing, settings.meterSmoothing);
  settings.meterPeakHoldMs = prefs_.getUInt(kMeterPeakHoldMs, settings.meterPeakHoldMs);
  settings.meterMuteRedDurationMs = prefs_.getUInt(kMeterMuteRedDurationMs, settings.meterMuteRedDurationMs);
  settings.lowEndstopEnabled = prefs_.getBool(kLowEndstopEnabled, settings.lowEndstopEnabled);
  settings.lowEndstopPosition = prefs_.getFloat(kLowEndstopPosition, settings.lowEndstopPosition);
  settings.lowEndstopStrength = prefs_.getFloat(kLowEndstopStrength, settings.lowEndstopStrength);
  settings.highEndstopEnabled = prefs_.getBool(kHighEndstopEnabled, settings.highEndstopEnabled);
  settings.highEndstopPosition = prefs_.getFloat(kHighEndstopPosition, settings.highEndstopPosition);
  settings.highEndstopStrength = prefs_.getFloat(kHighEndstopStrength, settings.highEndstopStrength);
  settings.normalize();

  tuning.detentStrengthMaxVPerRad = prefs_.getFloat(kDetentStrengthMaxVPerRad, tuning.detentStrengthMaxVPerRad);
  tuning.snapStrengthMaxVPerRad = prefs_.getFloat(kSnapStrengthMaxVPerRad, tuning.snapStrengthMaxVPerRad);
  tuning.clickPulseVoltage = prefs_.getFloat(kClickPulseVoltage, tuning.clickPulseVoltage);
  tuning.clickPulseMs = prefs_.getUInt(kClickPulseMs, tuning.clickPulseMs);
  tuning.endstopMinPos = prefs_.getFloat(kEndstopMinPos, tuning.endstopMinPos);
  tuning.endstopMaxPos = prefs_.getFloat(kEndstopMaxPos, tuning.endstopMaxPos);
  tuning.endstopMinStrength = prefs_.getFloat(kEndstopMinStrength, tuning.endstopMinStrength);
  tuning.endstopMaxStrength = prefs_.getFloat(kEndstopMaxStrength, tuning.endstopMaxStrength);
  tuning.debugStreamEnabled = prefs_.getBool(kDebugStreamEnabled, tuning.debugStreamEnabled);
  tuning.debugStreamIntervalMs = prefs_.getUInt(kDebugStreamIntervalMs, tuning.debugStreamIntervalMs);
  tuning.normalize();

  const bool hasDebugEndstops = prefs_.isKey(kEndstopMinPos) && prefs_.isKey(kEndstopMaxPos) &&
      prefs_.isKey(kEndstopMinStrength) && prefs_.isKey(kEndstopMaxStrength);
  if (!hasDebugEndstops) {
    syncEndstopsFromSettings(settings, tuning);
  }
}

bool SettingsStore::saveSettings(const NormalSettings& settings) {
  if (!started_ && !begin()) {
    return false;
  }

  NormalSettings normalized = settings;
  normalized.normalize();

  bool ok = true;
  ok = ok && prefs_.putBool(kAutoReconnectOnError, normalized.autoReconnectOnError);
  ok = ok && prefs_.putBool(kAutoConnectOnStartup, normalized.autoConnectOnStartup);
  ok = ok && prefs_.putFloat(kVolumeStepSize, normalized.volumeStepSize);
  ok = ok && prefs_.putInt(kDetentCount, normalized.detentCount);
  ok = ok && prefs_.putFloat(kDetentStrength, normalized.detentStrength);
  ok = ok && prefs_.putFloat(kSnapStrength, normalized.snapStrength);
  ok = ok && prefs_.putBool(kEncoderInvert, normalized.encoderInvert);
  ok = ok && prefs_.putFloat(kLedBrightness, normalized.ledBrightness);
  ok = ok && prefs_.putString(kMeterMode, meterModeToString(normalized.meterMode));
  ok = ok && prefs_.putString(kMeterColor, normalized.meterColor);
  ok = ok && prefs_.putFloat(kMeterBrightness, normalized.meterBrightness);
  ok = ok && prefs_.putFloat(kMeterSmoothing, normalized.meterSmoothing);
  ok = ok && prefs_.putUInt(kMeterPeakHoldMs, normalized.meterPeakHoldMs);
  ok = ok && prefs_.putUInt(kMeterMuteRedDurationMs, normalized.meterMuteRedDurationMs);
  ok = ok && prefs_.putBool(kLowEndstopEnabled, normalized.lowEndstopEnabled);
  ok = ok && prefs_.putFloat(kLowEndstopPosition, normalized.lowEndstopPosition);
  ok = ok && prefs_.putFloat(kLowEndstopStrength, normalized.lowEndstopStrength);
  ok = ok && prefs_.putBool(kHighEndstopEnabled, normalized.highEndstopEnabled);
  ok = ok && prefs_.putFloat(kHighEndstopPosition, normalized.highEndstopPosition);
  ok = ok && prefs_.putFloat(kHighEndstopStrength, normalized.highEndstopStrength);
  return ok;
}

bool SettingsStore::saveDebugTuning(const DebugTuning& tuning) {
  if (!started_ && !begin()) {
    return false;
  }

  DebugTuning normalized = tuning;
  normalized.normalize();

  bool ok = true;
  ok = ok && prefs_.putFloat(kDetentStrengthMaxVPerRad, normalized.detentStrengthMaxVPerRad);
  ok = ok && prefs_.putFloat(kSnapStrengthMaxVPerRad, normalized.snapStrengthMaxVPerRad);
  ok = ok && prefs_.putFloat(kClickPulseVoltage, normalized.clickPulseVoltage);
  ok = ok && prefs_.putUInt(kClickPulseMs, normalized.clickPulseMs);
  ok = ok && prefs_.putFloat(kEndstopMinPos, normalized.endstopMinPos);
  ok = ok && prefs_.putFloat(kEndstopMaxPos, normalized.endstopMaxPos);
  ok = ok && prefs_.putFloat(kEndstopMinStrength, normalized.endstopMinStrength);
  ok = ok && prefs_.putFloat(kEndstopMaxStrength, normalized.endstopMaxStrength);
  ok = ok && prefs_.putBool(kDebugStreamEnabled, normalized.debugStreamEnabled);
  ok = ok && prefs_.putUInt(kDebugStreamIntervalMs, normalized.debugStreamIntervalMs);
  return ok;
}

}  // namespace vp
