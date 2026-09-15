#pragma once

#include <Arduino.h>

struct Measurements {
  uint32_t timestampMs = 0;

  float ph = NAN;
  float tdsPpm = NAN;
  float turbidityNtu = NAN;
  float fluorideMgL = NAN;
  float flowLpm = 0.0F;
  float supplyV = NAN;

  float phVoltage = NAN;
  float tdsVoltage = NAN;
  float turbidityVoltage = NAN;
  float fluorideVoltage = NAN;

  bool phValid = false;
  bool tdsValid = false;
  bool turbidityValid = false;
  bool fluorideValid = false;
  bool supplyValid = false;
};
