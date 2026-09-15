#include <cassert>
#include <iostream>
#include "SafetyPolicy.h"

Measurements nominal() {
  Measurements m;
  m.ph = 7.2F;
  m.tdsPpm = 300.0F;
  m.turbidityNtu = 0.2F;
  m.flowLpm = 1.0F;
  m.supplyV = 12.2F;
  m.phValid = true;
  m.tdsValid = true;
  m.turbidityValid = true;
  m.fluorideValid = true;
  m.supplyValid = true;
  return m;
}

int main() {
  {
    const SafetyResult r = SafetyPolicy::evaluate(nominal());
    assert(r.safe());
    assert(!r.immediateLock);
  }
  {
    Measurements m = nominal();
    m.ph = 6.4F;
    const SafetyResult r = SafetyPolicy::evaluate(m);
    assert((r.faults & FAULT_PH_LIMIT) != 0);
  }
  {
    Measurements m = nominal();
    m.turbidityNtu = 1.0F;
    const SafetyResult r = SafetyPolicy::evaluate(m);
    assert((r.faults & FAULT_TURBIDITY_LIMIT) != 0);
  }
  {
    Measurements m = nominal();
    m.tdsPpm = 900.0F;
    const SafetyResult r = SafetyPolicy::evaluate(m);
    assert((r.faults & FAULT_TDS_INSTRUMENT_LIMIT) != 0);
  }
  {
    Measurements m = nominal();
    m.supplyV = 10.7F;
    const SafetyResult r = SafetyPolicy::evaluate(m);
    assert((r.faults & FAULT_LOW_SUPPLY) != 0);
  }
  {
    Measurements m = nominal();
    m.phValid = false;
    const SafetyResult r = SafetyPolicy::evaluate(m);
    assert((r.faults & FAULT_PH_SENSOR) != 0);
  }
  std::cout << "safety_policy_test: PASS\n";
}
