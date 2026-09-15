#pragma once

#include <Arduino.h>
#include <math.h>
#include "Config.h"
#include "Measurements.h"

extern volatile uint32_t gFlowPulseCount;

class SensorSuite {
 public:
  void begin(uint32_t nowMs) {
    pinMode(cfg::PIN_FLOW_PULSE, INPUT_PULLUP);
    lastFlowReadMs_ = nowMs;
  }

  bool update(uint32_t nowMs, Measurements& out) {
    if (nowMs - lastSampleMs_ < cfg::SENSOR_INTERVAL_MS) return false;
    lastSampleMs_ = nowMs;

    out.timestampMs = nowMs;
    out.phVoltage = readMedianVoltage(cfg::PIN_PH);
    out.tdsVoltage = readMedianVoltage(cfg::PIN_TDS);
    out.turbidityVoltage = readMedianVoltage(cfg::PIN_TURBIDITY);
    out.fluorideVoltage = readMedianVoltage(cfg::PIN_FLUORIDE);
    const float supplySenseV = readMedianVoltage(cfg::PIN_SUPPLY_VOLTAGE);

    out.ph = cfg::PH_SLOPE * out.phVoltage + cfg::PH_INTERCEPT;
    out.phValid = cfg::PH_CALIBRATION_VALID && inRange(out.ph, 0.0F, 14.0F);

    const float compensation = 1.0F + 0.02F * (cfg::WATER_TEMPERATURE_C - 25.0F);
    const float compensatedV = out.tdsVoltage / compensation;
    out.tdsPpm = (133.42F * compensatedV * compensatedV * compensatedV
                - 255.86F * compensatedV * compensatedV
                + 857.39F * compensatedV) * 0.5F * cfg::TDS_CELL_FACTOR;
    out.tdsValid = cfg::TDS_CALIBRATION_VALID && inRange(out.tdsPpm, 0.0F, 1000.0F);

    out.turbidityNtu = interpolateTurbidity(out.turbidityVoltage);
    out.turbidityValid = cfg::TURBIDITY_CALIBRATION_VALID
                      && inRange(out.turbidityNtu, 0.0F, 1000.0F);

    out.fluorideMgL = fluorideFromVoltage(out.fluorideVoltage);
    out.fluorideValid = !cfg::FLUORIDE_SENSOR_ENABLED
                     || (cfg::FLUORIDE_CALIBRATION_VALID
                         && inRange(out.fluorideMgL, 0.0F, 50.0F));

    out.supplyV = supplySenseV * cfg::SUPPLY_DIVIDER_RATIO;
    out.supplyValid = inRange(out.supplyV, 0.0F, 18.0F);
    out.flowLpm = readFlow(nowMs);
    return true;
  }

 private:
  uint32_t lastSampleMs_ = 0;
  uint32_t lastFlowReadMs_ = 0;

  static bool inRange(float value, float minimum, float maximum) {
    return isfinite(value) && value >= minimum && value <= maximum;
  }

  static float readMedianVoltage(uint8_t pin) {
    constexpr uint8_t count = 15;
    uint16_t samples[count];
    for (uint8_t i = 0; i < count; ++i) samples[i] = analogRead(pin);
    for (uint8_t i = 1; i < count; ++i) {
      const uint16_t key = samples[i];
      int8_t j = static_cast<int8_t>(i) - 1;
      while (j >= 0 && samples[j] > key) {
        samples[j + 1] = samples[j];
        --j;
      }
      samples[j + 1] = key;
    }
    return samples[count / 2] * cfg::ADC_REFERENCE_V / cfg::ADC_FULL_SCALE;
  }

  static float interpolateTurbidity(float voltage) {
    if (!isfinite(voltage)) return NAN;
    const uint8_t last = cfg::TURBIDITY_POINT_COUNT - 1;
    if (voltage >= cfg::TURBIDITY_V[0]) return cfg::TURBIDITY_NTU[0];
    if (voltage <= cfg::TURBIDITY_V[last]) return cfg::TURBIDITY_NTU[last];

    for (uint8_t i = 0; i < last; ++i) {
      const float highV = cfg::TURBIDITY_V[i];
      const float lowV = cfg::TURBIDITY_V[i + 1];
      if (voltage <= highV && voltage >= lowV) {
        const float fraction = (highV - voltage) / (highV - lowV);
        return cfg::TURBIDITY_NTU[i]
             + fraction * (cfg::TURBIDITY_NTU[i + 1] - cfg::TURBIDITY_NTU[i]);
      }
    }
    return NAN;
  }

  static float fluorideFromVoltage(float voltage) {
    if (!cfg::FLUORIDE_SENSOR_ENABLED || !isfinite(voltage)
        || fabs(cfg::F_SLOPE_MV_DECADE) < 1.0F) return NAN;
    const float electrodeMv = voltage * 1000.0F;
    const float log10MgL = (electrodeMv - cfg::F_1MG_MV) / cfg::F_SLOPE_MV_DECADE;
    return pow(10.0F, log10MgL);
  }

  float readFlow(uint32_t nowMs) {
    const uint32_t elapsedMs = nowMs - lastFlowReadMs_;
    if (elapsedMs == 0) return 0.0F;

    noInterrupts();
    const uint32_t pulses = gFlowPulseCount;
    gFlowPulseCount = 0;
    interrupts();

    lastFlowReadMs_ = nowMs;
    return (static_cast<float>(pulses) * 60000.0F)
         / (cfg::FLOW_PULSES_PER_LITRE * static_cast<float>(elapsedMs));
  }
};
