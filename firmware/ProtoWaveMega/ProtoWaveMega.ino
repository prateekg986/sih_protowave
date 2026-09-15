#include <avr/wdt.h>
#include <stdio.h>
#include "Actuators.h"
#include "Config.h"
#include "GsmNotifier.h"
#include "Measurements.h"
#include "SafetyPolicy.h"
#include "Sensors.h"

volatile uint32_t gFlowPulseCount = 0;

void flowPulseISR() { ++gFlowPulseCount; }

enum class SystemState : uint8_t {
  STARTUP_LOCKED,
  READY,
  UV_WARMUP,
  DISPENSING,
  FAULT_LOCKED,
  BACKWASH_ISOLATE,
  BACKWASH_REVERSE,
  BACKWASH_SETTLE
};

SensorSuite sensors;
Actuators actuators;
GsmNotifier gsm;
Measurements readings;
SafetyResult safety;

SystemState state = SystemState::STARTUP_LOCKED;
uint32_t stateSinceMs = 0;
uint32_t lastTelemetryMs = 0;
uint32_t lowFlowSinceMs = 0;
uint8_t unsafeSamples = 0;
uint8_t safeSamples = 0;
uint16_t lastFaults = FAULT_NONE;

const __FlashStringHelper* stateName(SystemState value) {
  switch (value) {
    case SystemState::STARTUP_LOCKED: return F("STARTUP_LOCKED");
    case SystemState::READY: return F("READY");
    case SystemState::UV_WARMUP: return F("UV_WARMUP");
    case SystemState::DISPENSING: return F("DISPENSING");
    case SystemState::FAULT_LOCKED: return F("FAULT_LOCKED");
    case SystemState::BACKWASH_ISOLATE: return F("BACKWASH_ISOLATE");
    case SystemState::BACKWASH_REVERSE: return F("BACKWASH_REVERSE");
    case SystemState::BACKWASH_SETTLE: return F("BACKWASH_SETTLE");
  }
  return F("UNKNOWN");
}

void enterState(SystemState next, uint32_t nowMs) {
  state = next;
  stateSinceMs = nowMs;

  switch (state) {
    case SystemState::STARTUP_LOCKED:
    case SystemState::FAULT_LOCKED: actuators.allClosed(); break;
    case SystemState::READY: actuators.serviceReady(); break;
    case SystemState::UV_WARMUP: actuators.uvWarmup(); break;
    case SystemState::DISPENSING: actuators.dispense(); break;
    case SystemState::BACKWASH_ISOLATE: actuators.isolate(); break;
    case SystemState::BACKWASH_REVERSE: actuators.reverseFlush(); break;
    case SystemState::BACKWASH_SETTLE: actuators.isolate(); break;
  }

  Serial.print(F("EVENT,state,"));
  Serial.println(stateName(state));
}

bool demandActive() { return digitalRead(cfg::PIN_DEMAND) == LOW; }
bool manualBackwashRequested() { return digitalRead(cfg::PIN_MANUAL_BACKWASH) == LOW; }

void processSafetySample(uint32_t nowMs) {
  safety = SafetyPolicy::evaluate(readings);

  if (safety.safe()) {
    unsafeSamples = 0;
    if (safeSamples < 255) ++safeSamples;
  } else {
    safeSamples = 0;
    if (unsafeSamples < 255) ++unsafeSamples;
  }

  const bool lockNow = safety.immediateLock
                    || unsafeSamples >= cfg::UNSAFE_SAMPLES_TO_LOCK;
  if (lockNow && state != SystemState::FAULT_LOCKED) {
    enterState(SystemState::FAULT_LOCKED, nowMs);
  }

  if (safety.faults != FAULT_NONE && safety.faults != lastFaults) {
    char message[161];
    snprintf(message, sizeof(message),
             "ProtoWave LOCK faults=0x%04X pH=%.2f turb=%.2fNTU TDS=%.0fppm F=%.2fmg/L V=%.1f",
             safety.faults, readings.ph, readings.turbidityNtu, readings.tdsPpm,
             readings.fluorideMgL, readings.supplyV);
    gsm.queue(message, nowMs);
  }
  lastFaults = safety.faults;

  if (state == SystemState::STARTUP_LOCKED
      && safeSamples >= cfg::SAFE_SAMPLES_TO_RECOVER) {
    enterState(SystemState::READY, nowMs);
  }

  // Recovery is deliberate: values must remain good for six samples and the
  // operator must release the demand control before water can flow again.
  if (state == SystemState::FAULT_LOCKED
      && safeSamples >= cfg::SAFE_SAMPLES_TO_RECOVER && !demandActive()) {
    enterState(SystemState::READY, nowMs);
  }
}

void checkForClogging(uint32_t nowMs) {
  if (state != SystemState::DISPENSING || !demandActive()) {
    lowFlowSinceMs = 0;
    return;
  }

  const float trigger = cfg::CLEAN_SERVICE_FLOW_LPM * cfg::BACKWASH_TRIGGER_FRACTION;
  if (readings.flowLpm < trigger) {
    if (lowFlowSinceMs == 0) lowFlowSinceMs = nowMs;
    if (nowMs - lowFlowSinceMs >= cfg::LOW_FLOW_PERSIST_MS) {
      enterState(SystemState::BACKWASH_ISOLATE, nowMs);
      lowFlowSinceMs = 0;
      gsm.queue("ProtoWave maintenance: low flow persisted; automatic backwash started.", nowMs);
    }
  } else {
    lowFlowSinceMs = 0;
  }
}

void runStateMachine(uint32_t nowMs) {
  if (state == SystemState::READY) {
    if (manualBackwashRequested()) {
      enterState(SystemState::BACKWASH_ISOLATE, nowMs);
    } else if (demandActive() && safety.safe()) {
      enterState(SystemState::UV_WARMUP, nowMs);
    }
  } else if (state == SystemState::UV_WARMUP) {
    if (!demandActive()) {
      enterState(SystemState::READY, nowMs);
    } else if (nowMs - stateSinceMs >= cfg::UV_WARMUP_MS) {
      enterState(SystemState::DISPENSING, nowMs);
    }
  } else if (state == SystemState::DISPENSING) {
    if (!demandActive()) enterState(SystemState::READY, nowMs);
  } else if (state == SystemState::BACKWASH_ISOLATE) {
    if (nowMs - stateSinceMs >= cfg::BACKWASH_ISOLATE_MS) {
      enterState(SystemState::BACKWASH_REVERSE, nowMs);
    }
  } else if (state == SystemState::BACKWASH_REVERSE) {
    if (nowMs - stateSinceMs >= cfg::BACKWASH_REVERSE_MS) {
      enterState(SystemState::BACKWASH_SETTLE, nowMs);
    }
  } else if (state == SystemState::BACKWASH_SETTLE) {
    if (nowMs - stateSinceMs >= cfg::BACKWASH_SETTLE_MS) {
      // A fresh demand action is required; do not auto-open the tap.
      enterState(safety.safe() ? SystemState::READY : SystemState::FAULT_LOCKED, nowMs);
    }
  }

  checkForClogging(nowMs);
  actuators.alarm(state == SystemState::FAULT_LOCKED && ((nowMs / 500UL) & 1U));
}

void printTelemetry(uint32_t nowMs) {
  if (nowMs - lastTelemetryMs < cfg::TELEMETRY_INTERVAL_MS) return;
  lastTelemetryMs = nowMs;

  Serial.print(F("DATA,"));
  Serial.print(nowMs);
  Serial.print(','); Serial.print(stateName(state));
  Serial.print(','); Serial.print(readings.ph, 2);
  Serial.print(','); Serial.print(readings.turbidityNtu, 2);
  Serial.print(','); Serial.print(readings.tdsPpm, 0);
  Serial.print(','); Serial.print(readings.fluorideMgL, 2);
  Serial.print(','); Serial.print(readings.flowLpm, 3);
  Serial.print(','); Serial.print(readings.supplyV, 2);
  Serial.print(F(",0x")); Serial.print(safety.faults, HEX);
  Serial.print(F(",0x")); Serial.println(safety.warnings, HEX);
}

void printStartupNotice() {
  Serial.println(F("ProtoWave Arduino Mega controller"));
  Serial.println(F("CSV: DATA,ms,state,pH,turbidity_NTU,TDS_ppm,fluoride_mgL,flow_Lpm,supply_V,faults,warnings"));
  if (!cfg::PH_CALIBRATION_VALID || !cfg::TDS_CALIBRATION_VALID
      || !cfg::TURBIDITY_CALIBRATION_VALID) {
    Serial.println(F("SAFETY LOCK: complete calibrations in Config.h before dispensing."));
  }
  if (!cfg::FLUORIDE_SENSOR_ENABLED) {
    Serial.println(F("LIMITED MONITORING: fluoride/heavy metals are not directly measured."));
  }
}

void setup() {
  wdt_disable();
  actuators.beginSafe();  // Fail closed before Serial or sensor initialization.
  pinMode(cfg::PIN_DEMAND, INPUT_PULLUP);
  pinMode(cfg::PIN_MANUAL_BACKWASH, INPUT_PULLUP);
  Serial.begin(115200);
  gsm.begin(Serial1, millis());
  sensors.begin(millis());
  attachInterrupt(digitalPinToInterrupt(cfg::PIN_FLOW_PULSE), flowPulseISR, FALLING);
  printStartupNotice();
  stateSinceMs = millis();
  wdt_enable(WDTO_8S);
}

void loop() {
  const uint32_t nowMs = millis();
  if (sensors.update(nowMs, readings)) processSafetySample(nowMs);
  runStateMachine(nowMs);
  gsm.tick(nowMs);
  printTelemetry(nowMs);
  wdt_reset();
}
