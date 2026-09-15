# ProtoWave exact-component Arduino Mega firmware

This version matches the latest component table exactly:

| Component | Module pin | Arduino/power connection |
|---|---|---|
| TS-300B | VCC / GND / A0 | Mega 5 V / Mega GND / Mega A0 |
| YF-S201 | Red / Black / Yellow | Mega 5 V / Mega GND / Mega D2 |
| Relay module | VCC / GND / IN1 | Mega 5 V / Mega GND / Mega D7 |
| SIM800L power | VCC / GND | Dedicated regulated 4.0 V / common GND |
| SIM800L UART | TX / RX | Mega RX1 D19 / Mega TX1 D18 through level conversion |

## Correct solenoid contact wiring

Use this unambiguous high-side arrangement:

1. Fused +12 V to relay COM.
2. Relay NO to solenoid positive.
3. Solenoid negative to 12 V GND.
4. 1N4007 directly across the solenoid: striped cathode to solenoid positive,
   anode to solenoid negative.
5. Never connect relay COM/NO, solenoid 12 V, or the diode power path to a
   Mega I/O pin or the Mega 5 V rail.

The low-side arrangement in the supplied table can also switch the valve, but
do not mix the two arrangements. The contact side of the relay is electrically
separate from IN1/VCC/GND.

## SIM800L warning

Do not power SIM800L from the Mega 5 V pin. Use a dedicated 4.0 V regulator
capable of 2 A current bursts and place a low-ESR capacitor close to the module
(at least 100 uF; 470-1000 uF is a practical prototype choice). Mega TX1 is 5 V
logic, so reduce it before SIM800L RX. A proper bidirectional level shifter is
preferred; at minimum, use the verified divider shown in your wiring plan on
Mega TX1 -> SIM RX. Grounds must be common.

## Before upload

1. Open `ProtoWaveExactComponents.ino` in Arduino IDE.
2. Select **Arduino Mega or Mega 2560**.
3. Keep the water output labelled **TEST WATER - NOT FOR DRINKING**.
4. Calibrate the TS-300B in the final opaque, bubble-free sensor cell using
   0, 1, 5 and 20 NTU standards.
5. Replace `CAL_VOLTAGE[]`, confirm the points descend with increasing NTU,
   then set `CALIBRATION_VALID = true`.
6. Calibrate the YF-S201 by collecting a known volume at several tank levels
   and update `FLOW_PULSES_PER_LITRE`.
7. Replace `ALERT_PHONE`, verify the 4.0 V supply and UART, then set
   `GSM_ENABLED = true`.
8. Verify whether the relay is active-low. Change `RELAY_ACTIVE_LOW` if needed.

## Operation

Open Serial Monitor at 115200 baud with newline enabled.

- `OPEN` opens the normally-closed valve only when calibration is valid and
  turbidity is below 1.0 NTU.
- `CLOSE` closes immediately.
- `STATUS` prints state, voltage, NTU, flow and warning status.
- `RESET` clears a latched fault only after six safe samples and no detected
  closed-valve flow.
- `TESTSMS` sends a test message only when GSM is enabled.

The valve closes on reset, invalid sensor data, three readings at or above
1.0 NTU, one reading at or above 5 NTU, persistent low flow while dispensing,
or persistent flow when the valve is closed.

## Honest limitations

- TS-300B is not reliable enough by itself to certify the 1 NTU drinking-water
  limit. The 1 NTU interlock is coded, but deployment requires a validated
  low-range turbidity instrument.
- YF-S201 is normally specified around 1-30 L/min. If the gravity bottle rig
  runs below 1 L/min, replace it with a calibrated low-flow sensor or disable
  the flow interlock during supervised bench testing.
- This component list measures turbidity and flow only. It does not measure pH,
  TDS, fluoride, heavy metals, bacteria or delivered UV dose.
- Your list has no physical demand input. This code uses USB Serial commands.
  Add an RFID-isolated output or a pushbutton before kiosk deployment.

