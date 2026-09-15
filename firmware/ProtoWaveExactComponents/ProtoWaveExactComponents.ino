/*
  ProtoWave - exact component firmware
  Board: Arduino Mega 2560

  Hardware in the supplied connection table:
    TS-300B turbidity module OUT -> A0
    YF-S201 flow pulse           -> D2
    Relay module IN1             -> D7
    SIM800L TX/RX                -> Serial1 RX1/TX1 (19/18)
    12 V normally-closed valve   -> relay contact circuit, never Mega power

  USB Serial commands (115200 baud):
    OPEN, CLOSE, STATUS, RESET, TESTSMS, HELP

  SAFETY: CALIBRATION_VALID is false on purpose. Replace the calibration
  points with measurements from real turbidity standards before setting it
  true. Until then, the solenoid remains closed.
*/

#include <Arduino.h>
#include <avr/wdt.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

namespace Config {

constexpr uint8_t TURBIDITY_PIN = A0;
constexpr uint8_t FLOW_PIN = 2;       // Mega external interrupt pin
constexpr uint8_t RELAY_PIN = 7;

// Most common relay boards energize when IN1 is LOW. Verify your module.
constexpr bool RELAY_ACTIVE_LOW = true;

constexpr float ADC_REFERENCE_V = 5.000F;  // Measure the real 5 V rail.
constexpr uint16_t ADC_MAX = 1023;

// ---- TS-300B calibration -------------------------------------------------
// These voltages are placeholders that define the code shape only.
// Replace them using 0, 1, 5 and 20 NTU standards in the final sensor cell.
// Clearer water normally gives the higher output voltage.
constexpr bool CALIBRATION_VALID = false;
constexpr uint8_t CAL_POINT_COUNT = 4;
constexpr float CAL_VOLTAGE[CAL_POINT_COUNT] = {4.20F, 4.05F, 3.80F, 3.30F};
constexpr float CAL_NTU[CAL_POINT_COUNT] = {0.0F, 1.0F, 5.0F, 20.0F};

// Conservative control points because UV-C follows the turbidity cell.
constexpr float TURBIDITY_WARNING_NTU = 0.80F;
constexpr float TURBIDITY_LOCK_NTU = 1.00F;
constexpr float TURBIDITY_IMMEDIATE_NTU = 5.00F;
constexpr uint8_t UNSAFE_SAMPLES_TO_LOCK = 3;
constexpr uint8_t SAFE_SAMPLES_TO_RESET = 6;

// YF-S201 nominal starting constant; calibrate by timed volume collection.
constexpr float FLOW_PULSES_PER_LITRE = 450.0F;
constexpr bool FLOW_INTERLOCK_ENABLED = true;
constexpr float MIN_DISPENSE_FLOW_LPM = 1.00F;
constexpr float CLOSED_VALVE_LEAK_FLOW_LPM = 0.20F;
constexpr uint32_t FLOW_STARTUP_GRACE_MS = 5000UL;
constexpr uint32_t LOW_FLOW_LOCK_DELAY_MS = 15000UL;
constexpr uint32_t LEAK_LOCK_DELAY_MS = 5000UL;

constexpr uint32_t SAMPLE_INTERVAL_MS = 1000UL;
constexpr uint32_t TELEMETRY_INTERVAL_MS = 5000UL;

// SIM800L needs a separate 4.0 V supply capable of 2 A bursts.
constexpr uint32_t GSM_BAUD = 9600UL;
constexpr bool GSM_ENABLED = false;
constexpr char ALERT_PHONE[] = "+910000000000";  // Replace before enabling.
constexpr uint32_t ALERT_COOLDOWN_MS = 30UL * 60UL * 1000UL;

}  // namespace Config

enum class State : uint8_t {
  CALIBRATION_LOCK,
  READY,
  DISPENSING,
  TURBIDITY_LOCK,
  FLOW_LOCK,
  LEAK_LOCK,
  SENSOR_LOCK
};

enum class SmsState : uint8_t {
  IDLE,
  SEND_AT,
  WAIT_AT,
  SEND_TEXT_MODE,
  WAIT_TEXT_MODE,
  SEND_RECIPIENT,
  WAIT_PROMPT,
  SEND_BODY,
  WAIT_RESULT
};

volatile uint32_t flowPulseCount = 0;

State state = State::CALIBRATION_LOCK;
bool valveOpen = false;
float turbidityVoltage = NAN;
float turbidityNtu = NAN;
float flowLpm = 0.0F;
bool turbidityValid = false;
uint8_t unsafeSamples = 0;
uint8_t safeSamples = 0;

uint32_t lastSampleMs = 0;
uint32_t lastTelemetryMs = 0;
uint32_t lastFlowReadMs = 0;
uint32_t dispenseStartedMs = 0;
uint32_t lowFlowStartedMs = 0;
uint32_t leakStartedMs = 0;

SmsState smsState = SmsState::IDLE;
uint32_t smsStateSinceMs = 0;
uint32_t lastAlertMs = UINT32_MAX - Config::ALERT_COOLDOWN_MS + 1UL;
char smsMessage[161] = {0};
char gsmResponse[96] = {0};
uint8_t gsmResponseLength = 0;

char commandBuffer[24] = {0};
uint8_t commandLength = 0;

void flowPulseIsr() {
  ++flowPulseCount;
}

const __FlashStringHelper* stateName(State value) {
  switch (value) {
    case State::CALIBRATION_LOCK: return F("CALIBRATION_LOCK");
    case State::READY: return F("READY");
    case State::DISPENSING: return F("DISPENSING");
    case State::TURBIDITY_LOCK: return F("TURBIDITY_LOCK");
    case State::FLOW_LOCK: return F("FLOW_LOCK");
    case State::LEAK_LOCK: return F("LEAK_LOCK");
    case State::SENSOR_LOCK: return F("SENSOR_LOCK");
  }
  return F("UNKNOWN");
}

void writeRelay(bool energize) {
  const bool high = Config::RELAY_ACTIVE_LOW ? !energize : energize;
  digitalWrite(Config::RELAY_PIN, high ? HIGH : LOW);
  valveOpen = energize;
}

void closeValve() {
  writeRelay(false);
}

void enterState(State next) {
  if (state == next) return;
  state = next;

  if (state != State::DISPENSING) closeValve();
  if (state == State::DISPENSING) {
    writeRelay(true);
    dispenseStartedMs = millis();
    lowFlowStartedMs = 0;
  }

  Serial.print(F("EVENT,state,"));
  Serial.println(stateName(state));
}

float medianVoltage(uint8_t pin) {
  constexpr uint8_t sampleCount = 21;
  uint16_t samples[sampleCount];

  for (uint8_t i = 0; i < sampleCount; ++i) {
    samples[i] = analogRead(pin);
  }

  for (uint8_t i = 1; i < sampleCount; ++i) {
    const uint16_t key = samples[i];
    int8_t j = static_cast<int8_t>(i) - 1;
    while (j >= 0 && samples[j] > key) {
      samples[j + 1] = samples[j];
      --j;
    }
    samples[j + 1] = key;
  }

  return samples[sampleCount / 2] * Config::ADC_REFERENCE_V / Config::ADC_MAX;
}

float voltageToNtu(float voltage) {
  if (!isfinite(voltage)) return NAN;
  const uint8_t last = Config::CAL_POINT_COUNT - 1;

  if (voltage >= Config::CAL_VOLTAGE[0]) return Config::CAL_NTU[0];
  if (voltage <= Config::CAL_VOLTAGE[last]) return Config::CAL_NTU[last];

  for (uint8_t i = 0; i < last; ++i) {
    const float highV = Config::CAL_VOLTAGE[i];
    const float lowV = Config::CAL_VOLTAGE[i + 1];
    if (voltage <= highV && voltage >= lowV) {
      const float fraction = (highV - voltage) / (highV - lowV);
      return Config::CAL_NTU[i]
           + fraction * (Config::CAL_NTU[i + 1] - Config::CAL_NTU[i]);
    }
  }
  return NAN;
}

float updateFlow(uint32_t nowMs) {
  const uint32_t elapsedMs = nowMs - lastFlowReadMs;
  if (elapsedMs == 0) return flowLpm;

  noInterrupts();
  const uint32_t pulses = flowPulseCount;
  flowPulseCount = 0;
  interrupts();

  lastFlowReadMs = nowMs;
  return (static_cast<float>(pulses) * 60000.0F)
       / (Config::FLOW_PULSES_PER_LITRE * static_cast<float>(elapsedMs));
}

void clearGsmResponse() {
  gsmResponseLength = 0;
  gsmResponse[0] = '\0';
}

void changeSmsState(SmsState next, uint32_t nowMs) {
  smsState = next;
  smsStateSinceMs = nowMs;
  clearGsmResponse();
}

bool gsmHas(const char* token) {
  return strstr(gsmResponse, token) != nullptr;
}

bool gsmTimedOut(uint32_t nowMs, uint32_t timeoutMs) {
  return nowMs - smsStateSinceMs >= timeoutMs;
}

void finishSms() {
  smsState = SmsState::IDLE;
  clearGsmResponse();
}

bool queueSms(const char* message, uint32_t nowMs, bool bypassCooldown = false) {
  if (!Config::GSM_ENABLED || smsState != SmsState::IDLE) return false;
  if (!bypassCooldown && nowMs - lastAlertMs < Config::ALERT_COOLDOWN_MS) return false;

  strncpy(smsMessage, message, sizeof(smsMessage) - 1);
  smsMessage[sizeof(smsMessage) - 1] = '\0';
  lastAlertMs = nowMs;
  changeSmsState(SmsState::SEND_AT, nowMs);
  return true;
}

void serviceGsm(uint32_t nowMs) {
  if (!Config::GSM_ENABLED) return;

  while (Serial1.available()) {
    const char c = static_cast<char>(Serial1.read());
    if (gsmResponseLength < sizeof(gsmResponse) - 1) {
      gsmResponse[gsmResponseLength++] = c;
      gsmResponse[gsmResponseLength] = '\0';
    }
  }

  switch (smsState) {
    case SmsState::IDLE:
      break;
    case SmsState::SEND_AT:
      Serial1.println(F("AT"));
      changeSmsState(SmsState::WAIT_AT, nowMs);
      break;
    case SmsState::WAIT_AT:
      if (gsmHas("OK")) changeSmsState(SmsState::SEND_TEXT_MODE, nowMs);
      else if (gsmTimedOut(nowMs, 2500UL)) finishSms();
      break;
    case SmsState::SEND_TEXT_MODE:
      Serial1.println(F("AT+CMGF=1"));
      changeSmsState(SmsState::WAIT_TEXT_MODE, nowMs);
      break;
    case SmsState::WAIT_TEXT_MODE:
      if (gsmHas("OK")) changeSmsState(SmsState::SEND_RECIPIENT, nowMs);
      else if (gsmTimedOut(nowMs, 3000UL)) finishSms();
      break;
    case SmsState::SEND_RECIPIENT:
      Serial1.print(F("AT+CMGS=\""));
      Serial1.print(Config::ALERT_PHONE);
      Serial1.println(F("\""));
      changeSmsState(SmsState::WAIT_PROMPT, nowMs);
      break;
    case SmsState::WAIT_PROMPT:
      if (gsmHas(">")) changeSmsState(SmsState::SEND_BODY, nowMs);
      else if (gsmTimedOut(nowMs, 5000UL)) finishSms();
      break;
    case SmsState::SEND_BODY:
      Serial1.print(smsMessage);
      Serial1.write(26);  // Ctrl+Z
      changeSmsState(SmsState::WAIT_RESULT, nowMs);
      break;
    case SmsState::WAIT_RESULT:
      if (gsmHas("+CMGS:")) finishSms();
      else if (gsmTimedOut(nowMs, 15000UL)) finishSms();
      break;
  }
}

void lockWithAlert(State faultState, const char* reason, uint32_t nowMs) {
  const bool newFault = state != faultState;
  enterState(faultState);
  if (newFault) queueSms(reason, nowMs);
}

void evaluateTurbidity(uint32_t nowMs) {
  turbidityVoltage = medianVoltage(Config::TURBIDITY_PIN);
  turbidityNtu = voltageToNtu(turbidityVoltage);

  const bool electricalRangeOk = turbidityVoltage > 0.05F && turbidityVoltage < 4.95F;
  turbidityValid = Config::CALIBRATION_VALID
                && electricalRangeOk
                && isfinite(turbidityNtu)
                && turbidityNtu >= 0.0F
                && turbidityNtu <= 1000.0F;

  if (!Config::CALIBRATION_VALID) {
    lockWithAlert(State::CALIBRATION_LOCK,
                  "ProtoWave locked: TS-300B calibration is not enabled.", nowMs);
    return;
  }

  if (!turbidityValid) {
    lockWithAlert(State::SENSOR_LOCK,
                  "ProtoWave locked: TS-300B signal invalid.", nowMs);
    return;
  }

  if (turbidityNtu >= Config::TURBIDITY_IMMEDIATE_NTU) {
    unsafeSamples = Config::UNSAFE_SAMPLES_TO_LOCK;
  } else if (turbidityNtu >= Config::TURBIDITY_LOCK_NTU) {
    if (unsafeSamples < 255) ++unsafeSamples;
  } else {
    unsafeSamples = 0;
  }

  if (turbidityNtu < Config::TURBIDITY_LOCK_NTU) {
    if (safeSamples < 255) ++safeSamples;
  } else {
    safeSamples = 0;
  }

  if (unsafeSamples >= Config::UNSAFE_SAMPLES_TO_LOCK) {
    lockWithAlert(State::TURBIDITY_LOCK,
                  "ProtoWave locked: treated-water turbidity is unsafe.", nowMs);
  }
}

void evaluateFlow(uint32_t nowMs) {
  if (!Config::FLOW_INTERLOCK_ENABLED) return;

  if (state == State::DISPENSING) {
    leakStartedMs = 0;
    if (nowMs - dispenseStartedMs < Config::FLOW_STARTUP_GRACE_MS) return;

    if (flowLpm < Config::MIN_DISPENSE_FLOW_LPM) {
      if (lowFlowStartedMs == 0) lowFlowStartedMs = nowMs;
      if (nowMs - lowFlowStartedMs >= Config::LOW_FLOW_LOCK_DELAY_MS) {
        lockWithAlert(State::FLOW_LOCK,
                      "ProtoWave locked: no/low flow while valve was open.", nowMs);
      }
    } else {
      lowFlowStartedMs = 0;
    }
    return;
  }

  lowFlowStartedMs = 0;
  if (flowLpm > Config::CLOSED_VALVE_LEAK_FLOW_LPM) {
    if (leakStartedMs == 0) leakStartedMs = nowMs;
    if (nowMs - leakStartedMs >= Config::LEAK_LOCK_DELAY_MS) {
      lockWithAlert(State::LEAK_LOCK,
                    "ProtoWave locked: flow detected with valve closed.", nowMs);
    }
  } else {
    leakStartedMs = 0;
  }
}

void printStatus() {
  Serial.print(F("DATA,state="));
  Serial.print(stateName(state));
  Serial.print(F(",valve="));
  Serial.print(valveOpen ? F("OPEN") : F("CLOSED"));
  Serial.print(F(",turbidity_V="));
  Serial.print(turbidityVoltage, 3);
  Serial.print(F(",turbidity_NTU="));
  Serial.print(turbidityNtu, 2);
  Serial.print(F(",flow_Lpm="));
  Serial.print(flowLpm, 3);
  Serial.print(F(",sensor_valid="));
  Serial.print(static_cast<uint16_t>(turbidityValid ? 1U : 0U));
  Serial.print(F(",warning="));
  Serial.println(static_cast<uint16_t>(
      turbidityValid && turbidityNtu >= Config::TURBIDITY_WARNING_NTU ? 1U : 0U));
}

void printHelp() {
  Serial.println(F("Commands: OPEN CLOSE STATUS RESET TESTSMS HELP"));
  Serial.println(F("OPEN succeeds only when calibrated sensor data are below 1.0 NTU."));
}

void uppercase(char* text) {
  for (; *text != '\0'; ++text) *text = static_cast<char>(toupper(*text));
}

void executeCommand(char* command, uint32_t nowMs) {
  uppercase(command);

  if (strcmp(command, "OPEN") == 0) {
    if (state == State::READY && turbidityValid
        && turbidityNtu < Config::TURBIDITY_LOCK_NTU) {
      enterState(State::DISPENSING);
    } else {
      Serial.println(F("DENIED: system is not READY with safe calibrated turbidity."));
    }
  } else if (strcmp(command, "CLOSE") == 0) {
    closeValve();
    if (state == State::DISPENSING) enterState(State::READY);
  } else if (strcmp(command, "STATUS") == 0) {
    printStatus();
  } else if (strcmp(command, "RESET") == 0) {
    if (!Config::CALIBRATION_VALID) {
      Serial.println(F("DENIED: calibration flag is false."));
    } else if (turbidityValid
               && turbidityNtu < Config::TURBIDITY_LOCK_NTU
               && safeSamples >= Config::SAFE_SAMPLES_TO_RESET
               && flowLpm <= Config::CLOSED_VALVE_LEAK_FLOW_LPM) {
      unsafeSamples = 0;
      enterState(State::READY);
    } else {
      Serial.println(F("DENIED: six safe samples and zero closed-valve flow required."));
    }
  } else if (strcmp(command, "TESTSMS") == 0) {
    if (!queueSms("ProtoWave GSM test message.", nowMs, true)) {
      Serial.println(F("SMS not queued. Check GSM_ENABLED, current SMS state and phone number."));
    }
  } else if (strcmp(command, "HELP") == 0) {
    printHelp();
  } else if (command[0] != '\0') {
    Serial.println(F("Unknown command. Type HELP."));
  }
}

void serviceUsbCommands(uint32_t nowMs) {
  while (Serial.available()) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\r' || c == '\n') {
      if (commandLength > 0) {
        commandBuffer[commandLength] = '\0';
        executeCommand(commandBuffer, nowMs);
        commandLength = 0;
      }
    } else if (commandLength < sizeof(commandBuffer) - 1) {
      commandBuffer[commandLength++] = c;
    }
  }
}

void setup() {
  wdt_disable();

  // Set the relay's inactive logic level before making the pin an output.
  digitalWrite(Config::RELAY_PIN, Config::RELAY_ACTIVE_LOW ? HIGH : LOW);
  pinMode(Config::RELAY_PIN, OUTPUT);
  closeValve();

  pinMode(Config::FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Config::FLOW_PIN), flowPulseIsr, FALLING);

  Serial.begin(115200);
  Serial1.begin(Config::GSM_BAUD);
  lastFlowReadMs = millis();

  Serial.println(F("ProtoWave exact-component controller / Arduino Mega 2560"));
  printHelp();
  if (!Config::CALIBRATION_VALID) {
    Serial.println(F("SAFETY LOCK: replace calibration points and set CALIBRATION_VALID=true."));
  }
  if (!Config::GSM_ENABLED) {
    Serial.println(F("GSM is disabled until its supply, number and UART are verified."));
  }

  wdt_enable(WDTO_8S);
}

void loop() {
  const uint32_t nowMs = millis();

  serviceUsbCommands(nowMs);
  serviceGsm(nowMs);

  if (nowMs - lastSampleMs >= Config::SAMPLE_INTERVAL_MS) {
    lastSampleMs = nowMs;
    flowLpm = updateFlow(nowMs);
    evaluateTurbidity(nowMs);

    // After calibration, collect six safe samples before becoming READY.
    if (state == State::CALIBRATION_LOCK && Config::CALIBRATION_VALID
        && turbidityValid && safeSamples >= Config::SAFE_SAMPLES_TO_RESET) {
      enterState(State::READY);
    }

    evaluateFlow(nowMs);
  }

  if (nowMs - lastTelemetryMs >= Config::TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = nowMs;
    printStatus();
  }

  wdt_reset();
}
