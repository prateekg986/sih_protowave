#pragma once

#include <Arduino.h>

namespace cfg {

// Arduino Mega 2560 pin map. Relays are assumed to be active LOW.
constexpr uint8_t PIN_FLOW_PULSE = 2;       // YF-S201 yellow wire
constexpr uint8_t PIN_DEMAND = 3;           // Active-low push button or RFID authorization output
constexpr uint8_t PIN_MANUAL_BACKWASH = 4;  // Active-low service button
constexpr uint8_t PIN_UV_ENABLE = 6;        // Logic-level MOSFET, active HIGH
constexpr uint8_t PIN_BUZZER = 7;
constexpr uint8_t PIN_RELAY_TAP = 8;        // Normally-closed dispensing solenoid
constexpr uint8_t PIN_RELAY_FEED = 9;       // Service-water isolation valve
constexpr uint8_t PIN_RELAY_BACKWASH = 10;  // Clean backwash-water valve
constexpr uint8_t PIN_RELAY_WASTE = 11;     // Dirty backwash-water drain valve

constexpr uint8_t PIN_PH = A0;
constexpr uint8_t PIN_TDS = A1;
constexpr uint8_t PIN_TURBIDITY = A2;
constexpr uint8_t PIN_SUPPLY_VOLTAGE = A3;
constexpr uint8_t PIN_FLUORIDE = A4;         // Optional conditioned fluoride ISE input

constexpr bool RELAY_ACTIVE_LOW = true;
constexpr float ADC_REFERENCE_V = 5.00F;     // Measure the real 5 V rail and update this value.
constexpr uint16_t ADC_FULL_SCALE = 1023;

// Never set these true until calibration has been completed with traceable standards.
constexpr bool PH_CALIBRATION_VALID = false;
constexpr bool TDS_CALIBRATION_VALID = false;
constexpr bool TURBIDITY_CALIBRATION_VALID = false;
constexpr bool FLUORIDE_SENSOR_ENABLED = false;
constexpr bool FLUORIDE_CALIBRATION_VALID = false;

// Two/three-point pH calibration: pH = slope * sensor_voltage + intercept.
// Replace both values with a regression from pH 4.00, 7.00 and 10.00 buffers.
constexpr float PH_SLOPE = -5.70F;
constexpr float PH_INTERCEPT = 21.34F;

// DFRobot-style TDS conversion is subsequently multiplied by this cell factor.
// Determine the factor with a certified conductivity/TDS standard at the sample temperature.
constexpr float TDS_CELL_FACTOR = 1.000F;
constexpr float WATER_TEMPERATURE_C = 25.0F;  // Add a real temperature probe for field use.

// Example array shape only. Replace the voltage values using 0, 1, 5 and 20 NTU
// formazin standards, then set TURBIDITY_CALIBRATION_VALID to true.
constexpr uint8_t TURBIDITY_POINT_COUNT = 4;
constexpr float TURBIDITY_V[TURBIDITY_POINT_COUNT] = {4.20F, 4.05F, 3.80F, 3.30F};
constexpr float TURBIDITY_NTU[TURBIDITY_POINT_COUNT] = {0.0F, 1.0F, 5.0F, 20.0F};

// Optional fluoride ISE amplifier calibration:
// electrode_mV = F_1MG_MV + F_SLOPE_MV_DECADE * log10(fluoride_mg_L / 1 mg/L)
// Replace these placeholders with two or more TISAB-conditioned standards.
constexpr float F_1MG_MV = 2500.0F;
constexpr float F_SLOPE_MV_DECADE = -59.16F;

// Common 0-25 V module uses an approximately 5:1 divider. Measure yours.
constexpr float SUPPLY_DIVIDER_RATIO = 5.000F;

// IS 10500-based operational boundaries and conservative control set-points.
constexpr float PH_LOCK_LOW = 6.50F;
constexpr float PH_LOCK_HIGH = 8.50F;
constexpr float PH_WARNING_LOW = 6.70F;
constexpr float PH_WARNING_HIGH = 8.30F;
constexpr float PH_IMMEDIATE_LOW = 5.00F;
constexpr float PH_IMMEDIATE_HIGH = 10.00F;

// With UV as the microbial barrier, 1 NTU is used as the interlock, not 5 NTU.
constexpr float TURBIDITY_WARNING_NTU = 0.80F;
constexpr float TURBIDITY_LOCK_NTU = 1.00F;
constexpr float TURBIDITY_IMMEDIATE_NTU = 5.00F;

constexpr float TDS_WARNING_PPM = 500.0F;
// The listed 0-1000 ppm board cannot verify the 2000 ppm BIS relaxation limit.
// Lock near the instrument ceiling because the true value is then unknown.
constexpr float TDS_INSTRUMENT_LIMIT_STOP_PPM = 900.0F;

constexpr float FLUORIDE_WARNING_MG_L = 1.00F;
// 1.4 mg/L leaves 0.1 mg/L guard-band below the 1.5 mg/L maximum.
constexpr float FLUORIDE_LOCK_MG_L = 1.40F;
constexpr float FLUORIDE_IMMEDIATE_MG_L = 2.00F;

constexpr float SUPPLY_WARNING_V = 11.20F;
constexpr float SUPPLY_LOCK_V = 10.80F;

constexpr uint32_t SENSOR_INTERVAL_MS = 5000UL;
constexpr uint32_t TELEMETRY_INTERVAL_MS = 10000UL;
constexpr uint8_t UNSAFE_SAMPLES_TO_LOCK = 3;
constexpr uint8_t SAFE_SAMPLES_TO_RECOVER = 6;

// Flow must be calibrated gravimetrically. 450 pulses/L is only a starting value.
constexpr float FLOW_PULSES_PER_LITRE = 450.0F;
constexpr float CLEAN_SERVICE_FLOW_LPM = 1.00F;
constexpr float BACKWASH_TRIGGER_FRACTION = 0.60F;
constexpr uint32_t LOW_FLOW_PERSIST_MS = 30000UL;

constexpr uint32_t UV_WARMUP_MS = 5000UL;
constexpr uint32_t BACKWASH_ISOLATE_MS = 3000UL;
constexpr uint32_t BACKWASH_REVERSE_MS = 60000UL;
constexpr uint32_t BACKWASH_SETTLE_MS = 15000UL;
constexpr uint32_t ALERT_COOLDOWN_MS = 30UL * 60UL * 1000UL;

// SIM800L on Serial1: Mega TX1 pin 18 -> level shifter -> SIM RX;
// SIM TX -> level shifter -> Mega RX1 pin 19.
constexpr uint32_t GSM_BAUD = 9600UL;
constexpr bool GSM_ENABLED = false;
constexpr bool GSM_VOICE_CALL_ENABLED = false;
constexpr char ALERT_PHONE[] = "+910000000000";  // Replace; do not enable GSM before this.

}  // namespace cfg
