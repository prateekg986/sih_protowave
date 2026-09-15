# ProtoWave Arduino Mega firmware

This sketch is deliberately fail-closed. On a new build the three calibration
flags in `Config.h` are `false`, so the drinking-water solenoid cannot open.
Calibrate the sensors, replace the coefficients, verify them against reference
instruments, and only then set the corresponding flags to `true`.

## Arduino IDE

1. Open `ProtoWaveMega.ino` in Arduino IDE 2.x.
2. Select **Arduino Mega or Mega 2560** and the correct serial port.
3. Replace the calibration values and the alert telephone number in `Config.h`.
4. Leave `GSM_ENABLED = false` during bench testing.
5. Upload, open Serial Monitor at 115200 baud, and verify the CSV readings.

No third-party Arduino libraries are required.

## Operating logic

- Power-up and any invalid sensor state close all water valves.
- A push button or RFID controller pulls `PIN_DEMAND` low.
- The UV unit warms up before the normally-closed tap solenoid is energized.
- Three consecutive non-emergency unsafe samples cause a lock; extreme values
  lock immediately. Recovery requires six safe samples and a released demand.
- Flow below 60% of the calibrated clean-bed baseline for 30 seconds during an
  actual dispense request starts the reverse-flush sequence.
- Backwash closes feed and drinking-water paths, then opens clean backwash and
  waste paths. It never resumes dispensing automatically.
- Alerts are rate limited and use fixed buffers rather than dynamic `String`.

## Important limitations

- The TS-300B and 0-1000 ppm TDS board are prototype/trend sensors, not
  compliance instruments.
- TDS is not a measurement of fluoride or any individual heavy metal.
- Direct fluoride interlocking requires a fluoride ISE, high-impedance
  amplifier, TISAB-conditioned side-stream cell, and calibration.
- Heavy-metal claims require periodic AAS/ICP-MS or a separately validated
  stripping-voltammetry module.
- A YF-S201 is specified for roughly 1-30 L/min. If the bottle rig runs below
  1 L/min, use a calibrated low-flow sensor and update `FLOW_PULSES_PER_LITRE`.
