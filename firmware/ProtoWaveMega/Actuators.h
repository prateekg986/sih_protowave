#pragma once

#include <Arduino.h>
#include "Config.h"

class Actuators {
 public:
  void beginSafe() {
    safeOutput(cfg::PIN_RELAY_TAP);
    safeOutput(cfg::PIN_RELAY_FEED);
    safeOutput(cfg::PIN_RELAY_BACKWASH);
    safeOutput(cfg::PIN_RELAY_WASTE);
    pinMode(cfg::PIN_UV_ENABLE, OUTPUT);
    pinMode(cfg::PIN_BUZZER, OUTPUT);
    allClosed();
  }

  void allClosed() {
    tap(false);
    feed(false);
    backwash(false);
    waste(false);
    uv(false);
    buzzer(false);
  }

  void serviceReady() {
    tap(false);
    feed(true);
    backwash(false);
    waste(false);
    uv(false);
  }

  void uvWarmup() {
    tap(false);
    feed(true);
    backwash(false);
    waste(false);
    uv(true);
  }

  void dispense() {
    feed(true);
    backwash(false);
    waste(false);
    uv(true);
    tap(true);
  }

  void isolate() { allClosed(); }

  void reverseFlush() {
    tap(false);
    feed(false);
    backwash(true);
    waste(true);
    uv(false);
  }

  void alarm(bool enabled) { buzzer(enabled); }

 private:
  static void safeOutput(uint8_t pin) {
    digitalWrite(pin, cfg::RELAY_ACTIVE_LOW ? HIGH : LOW);
    pinMode(pin, OUTPUT);
  }

  static void relay(uint8_t pin, bool energized) {
    const bool level = cfg::RELAY_ACTIVE_LOW ? !energized : energized;
    digitalWrite(pin, level ? HIGH : LOW);
  }

  static void tap(bool open) { relay(cfg::PIN_RELAY_TAP, open); }
  static void feed(bool open) { relay(cfg::PIN_RELAY_FEED, open); }
  static void backwash(bool open) { relay(cfg::PIN_RELAY_BACKWASH, open); }
  static void waste(bool open) { relay(cfg::PIN_RELAY_WASTE, open); }
  static void uv(bool enabled) { digitalWrite(cfg::PIN_UV_ENABLE, enabled ? HIGH : LOW); }
  static void buzzer(bool enabled) { digitalWrite(cfg::PIN_BUZZER, enabled ? HIGH : LOW); }
};
