# ProtoWave Arduino Mega design package

This package converts the SIH26040 concept into a buildable, fail-closed
Arduino Mega prototype. The recommended physical arrangement is a descending
gravity staircase, not a level row of bottles.

## Start here

1. Read `report/ProtoWave_Arduino_Technical_Design.pdf`.
2. Fabricate from `visuals/equipment-placement-schematic.svg`; the labelled
   photorealistic render is a presentation aid, not a piping drawing.
3. For the exact latest component list, upload
   `firmware/ProtoWaveExactComponents/ProtoWaveExactComponents.ino` and follow
   its README. `firmware/ProtoWaveMega` is the advanced four-valve version for
   the later pH/TDS/fluoride/backwash expansion.
4. Calibrate every sensor. The firmware intentionally keeps the tap locked
   while the calibration flags are false.
5. Validate treated water against a certified laboratory before anyone drinks it.

## Why the bottles descend

The available driving pressure is set by the vertical difference between the
raw-water surface and the outlet. One metre of water head is approximately
9.8 kPa (0.098 bar). A staircase does not add pressure beyond that total drop;
it avoids wasting head on uphill tubing, reduces air locks, and lets every
vented stage drain predictably into the next one.

Use a raw-water surface at least 1.0-1.5 m above the tap for the demonstrator,
short tubing, generous tube bore, and a vent at the top of every bottle. Measure
the actual flow with clean and dirty media. If the rig operates below 1 L/min,
the listed YF-S201 is outside its normal range; use a calibrated low-flow meter.

## Treatment order

0. Raw-water reservoir and sampling port.
1. Washed coarse gravel and graded quartz sand for suspended solids.
2. High-calcium limestone for acidic mine water; bypass it when raw pH is
   already neutral or alkaline.
3. Replaceable natural-zeolite and biochar cartridges for polishing.
4. Activated alumina or a validated calcium-modified sorbent for fluoride.
5. Opaque, bubble-free sensor cell: pH, TDS, then turbidity.
6. Flow sensor, validated closed UV-C reactor, normally-closed tap solenoid.

The fluoride and metals media must remain modular. High-fluoride groundwater
and acid mine drainage are different water chemistries and should not be
presented as one universal fixed bed.

## Non-negotiable limitations

- TDS is not a fluoride sensor and is not a heavy-metal sensor.
- TS-300B is useful for a trend demonstration, but is not suitable for proving
  the 1 NTU drinking-water target without independent low-range validation.
- Direct fluoride control needs a fluoride ISE or validated colorimetric method.
- Heavy-metal claims need certified laboratory testing or a separately validated
  electrochemical analyzer.
- A UV-C chamber needs validated delivered dose and an intensity/current
  interlock. An LED turning on does not prove disinfection.
- Food-grade PET bottles are suitable for a short supervised prototype only.
  Deployment needs opaque, cleanable, pressure-rated housings.

## Firmware

Both sketches use no third-party libraries and are fail-closed. The exact-list
version controls one solenoid from TS-300B turbidity, monitors YF-S201 flow and
sends SIM800L alerts. The advanced version adds pH/TDS/fluoride inputs, UV
warm-up and a four-valve backwash state machine.
"# sih_protowave" 
