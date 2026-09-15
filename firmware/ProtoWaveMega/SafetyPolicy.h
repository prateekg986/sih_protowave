#pragma once

#include <Arduino.h>
#include <math.h>
#include "Config.h"
#include "Measurements.h"

enum FaultBit : uint16_t {
  FAULT_NONE = 0,
  FAULT_PH_SENSOR = 1U << 0,
  FAULT_TDS_SENSOR = 1U << 1,
  FAULT_TURBIDITY_SENSOR = 1U << 2,
  FAULT_FLUORIDE_SENSOR = 1U << 3,
  FAULT_SUPPLY_SENSOR = 1U << 4,
  FAULT_PH_LIMIT = 1U << 5,
  FAULT_TURBIDITY_LIMIT = 1U << 6,
  FAULT_TDS_INSTRUMENT_LIMIT = 1U << 7,
  FAULT_FLUORIDE_LIMIT = 1U << 8,
  FAULT_LOW_SUPPLY = 1U << 9
};

enum WarningBit : uint16_t {
  WARNING_NONE = 0,
  WARNING_PH_MARGIN = 1U << 0,
  WARNING_TURBIDITY = 1U << 1,
  WARNING_TDS = 1U << 2,
  WARNING_FLUORIDE = 1U << 3,
  WARNING_LOW_SUPPLY = 1U << 4
};

struct SafetyResult {
  uint16_t faults = FAULT_NONE;
  uint16_t warnings = WARNING_NONE;
  bool immediateLock = false;

  bool safe() const { return faults == FAULT_NONE; }
};

class SafetyPolicy {
 public:
  static SafetyResult evaluate(const Measurements& m) {
    SafetyResult result;

    if (!m.phValid) result.faults |= FAULT_PH_SENSOR;
    if (!m.tdsValid) result.faults |= FAULT_TDS_SENSOR;
    if (!m.turbidityValid) result.faults |= FAULT_TURBIDITY_SENSOR;
    if (cfg::FLUORIDE_SENSOR_ENABLED && !m.fluorideValid) {
      result.faults |= FAULT_FLUORIDE_SENSOR;
    }
    if (!m.supplyValid) result.faults |= FAULT_SUPPLY_SENSOR;

    if (m.phValid) {
      if (m.ph < cfg::PH_LOCK_LOW || m.ph > cfg::PH_LOCK_HIGH) {
        result.faults |= FAULT_PH_LIMIT;
      } else if (m.ph < cfg::PH_WARNING_LOW || m.ph > cfg::PH_WARNING_HIGH) {
        result.warnings |= WARNING_PH_MARGIN;
      }
      if (m.ph < cfg::PH_IMMEDIATE_LOW || m.ph > cfg::PH_IMMEDIATE_HIGH) {
        result.immediateLock = true;
      }
    }

    if (m.turbidityValid) {
      if (m.turbidityNtu >= cfg::TURBIDITY_LOCK_NTU) {
        result.faults |= FAULT_TURBIDITY_LIMIT;
      } else if (m.turbidityNtu >= cfg::TURBIDITY_WARNING_NTU) {
        result.warnings |= WARNING_TURBIDITY;
      }
      if (m.turbidityNtu >= cfg::TURBIDITY_IMMEDIATE_NTU) {
        result.immediateLock = true;
      }
    }

    if (m.tdsValid) {
      if (m.tdsPpm >= cfg::TDS_INSTRUMENT_LIMIT_STOP_PPM) {
        result.faults |= FAULT_TDS_INSTRUMENT_LIMIT;
      } else if (m.tdsPpm > cfg::TDS_WARNING_PPM) {
        result.warnings |= WARNING_TDS;
      }
    }

    if (cfg::FLUORIDE_SENSOR_ENABLED && m.fluorideValid) {
      if (m.fluorideMgL >= cfg::FLUORIDE_LOCK_MG_L) {
        result.faults |= FAULT_FLUORIDE_LIMIT;
      } else if (m.fluorideMgL >= cfg::FLUORIDE_WARNING_MG_L) {
        result.warnings |= WARNING_FLUORIDE;
      }
      if (m.fluorideMgL >= cfg::FLUORIDE_IMMEDIATE_MG_L) {
        result.immediateLock = true;
      }
    }

    if (m.supplyValid) {
      if (m.supplyV < cfg::SUPPLY_LOCK_V) {
        result.faults |= FAULT_LOW_SUPPLY;
      } else if (m.supplyV < cfg::SUPPLY_WARNING_V) {
        result.warnings |= WARNING_LOW_SUPPLY;
      }
    }
    return result;
  }
};
