# Wiring Guide

## Overview

This guide covers wiring the ESP-32 to 6x A4988/DRV8825 stepper motor drivers for a 6-axis robotic arm.

## Components Required

- 1x ESP-32 DevKit (ESP-WROOM-32)
- 6x A4988 or DRV8825 stepper driver modules
- 6x NEMA 17 stepper motors (or similar)
- 1x 12V/24V power supply (depending on motors)
- Capacitors: 100µF electrolytic per driver (recommended)
- Wiring and connectors

## Pin Assignments

Default pin configuration (can be changed in `config.h`):

| Joint | Function | Step Pin | Dir Pin | Description |
|-------|----------|----------|---------|-------------|
| J1 | Base rotation | GPIO 2 | GPIO 4 | Base/turntable |
| J2 | Shoulder | GPIO 16 | GPIO 17 | First arm segment |
| J3 | Elbow | GPIO 18 | GPIO 19 | Second arm segment |
| J4 | Wrist pitch | GPIO 21 | GPIO 22 | Wrist up/down |
| J5 | Wrist roll | GPIO 23 | GPIO 25 | Wrist rotation |
| J6 | End effector | GPIO 26 | GPIO 27 | Gripper/tool |

**Shared pins:**
- Enable: GPIO 15 (active LOW, shared by all drivers)

## Wiring Diagram

```
                    ESP-32 DevKit
                   +-------------+
                   |             |
    J1 Step -------|GPIO 2       |
    J1 Dir  -------|GPIO 4       |
                   |             |
    J2 Step -------|GPIO 16      |
    J2 Dir  -------|GPIO 17      |
                   |             |
    J3 Step -------|GPIO 18      |
    J3 Dir  -------|GPIO 19      |
                   |             |
    J4 Step -------|GPIO 21      |
    J4 Dir  -------|GPIO 22      |
                   |             |
    J5 Step -------|GPIO 23      |
    J5 Dir  -------|GPIO 25      |
                   |             |
    J6 Step -------|GPIO 26      |
    J6 Dir  -------|GPIO 27      |
                   |             |
    All Enable ----|GPIO 15      |
                   |             |
    GND -----------|GND          |
                   +-------------+
```

## A4988/DRV8825 Driver Wiring

Each driver module should be wired as follows:

```
A4988/DRV8825 Driver
+------------------+
|                  |
| VMOT ----+------ | 12V/24V Power Supply (+)
| GND  ----|------ | Power Supply (-)
|          |       |
|          +--[100µF Capacitor]--+
|                                |
| VDD  --------- | ESP-32 3.3V   |
| GND  --------- | ESP-32 GND    |
|                                |
| STEP --------- | ESP-32 Step Pin (see table)
| DIR  --------- | ESP-32 Dir Pin (see table)
| EN   --------- | ESP-32 GPIO 15 (Enable)
|                |
| MS1  --------- | (Microstepping - see below)
| MS2  --------- | (Microstepping - see below)
| MS3  --------- | (Microstepping - see below)
|                |
| 1A   --------- | Motor Coil A+
| 1B   --------- | Motor Coil A-
| 2A   --------- | Motor Coil B+
| 2B   --------- | Motor Coil B-
|                |
+----------------+
```

## Microstepping Configuration

Connect MS1, MS2, MS3 to set microstepping resolution:

### A4988
| MS1 | MS2 | MS3 | Resolution |
|-----|-----|-----|------------|
| LOW | LOW | LOW | Full step |
| HIGH | LOW | LOW | 1/2 step |
| LOW | HIGH | LOW | 1/4 step |
| HIGH | HIGH | LOW | 1/8 step |
| HIGH | HIGH | HIGH | 1/16 step |

### DRV8825
| MS1 | MS2 | MS3 | Resolution |
|-----|-----|-----|------------|
| LOW | LOW | LOW | Full step |
| HIGH | LOW | LOW | 1/2 step |
| LOW | HIGH | LOW | 1/4 step |
| HIGH | HIGH | LOW | 1/8 step |
| LOW | LOW | HIGH | 1/16 step |
| HIGH | HIGH | HIGH | 1/32 step |

**Recommendation:** Start with full step for testing, then increase resolution for smoother motion.

## Current Limiting

**IMPORTANT:** Set the current limit on each driver before connecting motors!

1. Calculate: VREF = (I_motor × 8 × R_sense) for A4988
2. Typical R_sense = 0.1Ω for most modules
3. Use a multimeter to measure and adjust the potentiometer
4. Start low (50% of motor rating) and increase as needed

## Power Supply Notes

- Use a separate power supply for motors (12V or 24V)
- The ESP-32 can be powered via USB during development
- Add a bulk capacitor (100µF or more) across the power rails
- Consider a kill switch for safety

## Safety Considerations

1. **Never connect/disconnect motors while powered**
2. Add an emergency stop button connected to GPIO 34 (optional)
3. Ensure proper heat dissipation for drivers
4. Use appropriate wire gauge for motor current
5. Consider current-limiting fuses

## Troubleshooting

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| Motor doesn't move | EN pin not LOW | Check enable signal |
| Motor vibrates but doesn't turn | Wrong coil wiring | Swap one coil pair |
| Motor gets very hot | Current too high | Reduce VREF setting |
| Jerky motion | Acceleration too high | Lower acceleration in config.h |
| ESP-32 resets | Power supply issue | Add capacitors, check wiring |
