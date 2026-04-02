#include "config/AppConfig.h"

namespace vp {

const DeviceConfig& getDeviceConfig() {
  static const DeviceConfig config{};
  return config;
}

}  // namespace vp
