#pragma once

#include <cstdint>
#include <cmath>
#include <cstddef>

using uint8_t = std::uint8_t;
using uint16_t = std::uint16_t;
using uint32_t = std::uint32_t;

constexpr uint8_t A0 = 54;
constexpr uint8_t A1 = 55;
constexpr uint8_t A2 = 56;
constexpr uint8_t A3 = 57;
constexpr uint8_t A4 = 58;

constexpr uint8_t LOW = 0;
constexpr uint8_t HIGH = 1;
constexpr uint8_t INPUT_PULLUP = 2;
constexpr uint8_t OUTPUT = 1;
constexpr int FALLING = 2;
constexpr int HEX = 16;

class __FlashStringHelper;
#define F(text) reinterpret_cast<const __FlashStringHelper*>(text)

class HardwareSerial {
 public:
  void begin(uint32_t) {}
  int available() { return 0; }
  int read() { return -1; }
  std::size_t write(uint8_t) { return 1; }
  std::size_t print(const char*) { return 1; }
  std::size_t print(const __FlashStringHelper*) { return 1; }
  std::size_t print(char) { return 1; }
  std::size_t print(float, int = 2) { return 1; }
  std::size_t print(uint32_t, int = 10) { return 1; }
  std::size_t print(uint16_t, int = 10) { return 1; }
  std::size_t println() { return 1; }
  std::size_t println(const char*) { return 1; }
  std::size_t println(const __FlashStringHelper*) { return 1; }
  std::size_t println(uint16_t, int = 10) { return 1; }
};

extern HardwareSerial Serial;
extern HardwareSerial Serial1;

inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t) {}
inline int digitalRead(uint8_t) { return HIGH; }
inline int analogRead(uint8_t) { return 512; }
inline void noInterrupts() {}
inline void interrupts() {}
inline uint32_t millis() { return 0; }
inline int digitalPinToInterrupt(uint8_t pin) { return pin; }
inline void attachInterrupt(int, void (*)(), int) {}
