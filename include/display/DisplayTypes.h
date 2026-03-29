#pragma once

#include <Arduino.h>

namespace vp {

struct DisplayModel {
  String screen;
  String title;
  String subtitle;
  String valueText;
  String iconRef;
  bool muted = false;
  String accent;
};

}  // namespace vp