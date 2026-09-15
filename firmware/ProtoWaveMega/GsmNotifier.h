#pragma once

#include <Arduino.h>
#include <string.h>
#include "Config.h"

class GsmNotifier {
 public:
  void begin(HardwareSerial& modem, uint32_t nowMs) {
    modem_ = &modem;
    modem_->begin(cfg::GSM_BAUD);
    stateSinceMs_ = nowMs;
  }

  bool queue(const char* message, uint32_t nowMs) {
    if (!cfg::GSM_ENABLED || modem_ == nullptr || state_ != State::IDLE
        || nowMs - lastAlertMs_ < cfg::ALERT_COOLDOWN_MS) return false;
    strncpy(message_, message, sizeof(message_) - 1);
    message_[sizeof(message_) - 1] = '\0';
    lastAlertMs_ = nowMs;
    transition(State::SEND_AT, nowMs);
    return true;
  }

  void tick(uint32_t nowMs) {
    if (!cfg::GSM_ENABLED || modem_ == nullptr) return;
    drainResponse();

    switch (state_) {
      case State::IDLE: return;
      case State::SEND_AT:
        sendLine(F("AT"));
        transition(State::WAIT_AT, nowMs);
        break;
      case State::WAIT_AT:
        if (hasToken("OK") || timedOut(nowMs, 2500UL)) transition(State::SEND_TEXT_MODE, nowMs);
        break;
      case State::SEND_TEXT_MODE:
        sendLine(F("AT+CMGF=1"));
        transition(State::WAIT_TEXT_MODE, nowMs);
        break;
      case State::WAIT_TEXT_MODE:
        if (hasToken("OK")) transition(State::SEND_RECIPIENT, nowMs);
        else if (timedOut(nowMs, 3000UL)) finish();
        break;
      case State::SEND_RECIPIENT:
        modem_->print(F("AT+CMGS=\""));
        modem_->print(cfg::ALERT_PHONE);
        modem_->println(F("\""));
        clearResponse();
        transition(State::WAIT_PROMPT, nowMs);
        break;
      case State::WAIT_PROMPT:
        if (hasToken(">")) transition(State::SEND_BODY, nowMs);
        else if (timedOut(nowMs, 5000UL)) finish();
        break;
      case State::SEND_BODY:
        modem_->print(message_);
        modem_->write(26);
        clearResponse();
        transition(State::WAIT_SMS_RESULT, nowMs);
        break;
      case State::WAIT_SMS_RESULT:
        if (hasToken("+CMGS:")) {
          transition(cfg::GSM_VOICE_CALL_ENABLED ? State::DIAL : State::DONE, nowMs);
        } else if (timedOut(nowMs, 15000UL)) {
          finish();
        }
        break;
      case State::DIAL:
        modem_->print(F("ATD"));
        modem_->print(cfg::ALERT_PHONE);
        modem_->println(F(";"));
        transition(State::CALL_ACTIVE, nowMs);
        break;
      case State::CALL_ACTIVE:
        if (nowMs - stateSinceMs_ >= 20000UL) transition(State::HANG_UP, nowMs);
        break;
      case State::HANG_UP:
        sendLine(F("ATH"));
        transition(State::DONE, nowMs);
        break;
      case State::DONE:
        if (nowMs - stateSinceMs_ >= 500UL) finish();
        break;
    }
  }

 private:
  enum class State : uint8_t {
    IDLE, SEND_AT, WAIT_AT, SEND_TEXT_MODE, WAIT_TEXT_MODE, SEND_RECIPIENT,
    WAIT_PROMPT, SEND_BODY, WAIT_SMS_RESULT, DIAL, CALL_ACTIVE, HANG_UP, DONE
  };

  HardwareSerial* modem_ = nullptr;
  State state_ = State::IDLE;
  uint32_t stateSinceMs_ = 0;
  uint32_t lastAlertMs_ = 0 - cfg::ALERT_COOLDOWN_MS;
  char message_[161] = {0};
  char response_[96] = {0};
  uint8_t responseLength_ = 0;

  void drainResponse() {
    while (modem_->available()) {
      const char c = static_cast<char>(modem_->read());
      if (responseLength_ < sizeof(response_) - 1) {
        response_[responseLength_++] = c;
        response_[responseLength_] = '\0';
      }
    }
  }

  bool hasToken(const char* token) const { return strstr(response_, token) != nullptr; }
  bool timedOut(uint32_t nowMs, uint32_t timeoutMs) const {
    return nowMs - stateSinceMs_ >= timeoutMs;
  }

  void clearResponse() {
    responseLength_ = 0;
    response_[0] = '\0';
  }

  void transition(State next, uint32_t nowMs) {
    state_ = next;
    stateSinceMs_ = nowMs;
    clearResponse();
  }

  void finish() {
    state_ = State::IDLE;
    clearResponse();
  }

  void sendLine(const __FlashStringHelper* line) {
    modem_->println(line);
    clearResponse();
  }
};
