# Roboarm

```
     ____       __          ___
    / __ \___  / /_  ____  /   |  _________ ___
   / /_/ / _ \/ __ \/ __ \/ /| | / ___/ __ `__ \
  / _, _/  __/ /_/ / /_/ / ___ |/ /  / / / / / /
 /_/ |_|\___/_.___/\____/_/  |_/_/  /_/ /_/ /_/

 6-Axis Robotic Arm Controller for ESP-32
```

Build your own robot arm and control it with WiFi or Serial commands!

[![Build Status](https://github.com/banton/roboarm/actions/workflows/ci.yml/badge.svg)](https://github.com/banton/roboarm/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## What is This?

Roboarm is a complete firmware and host toolkit for building a **6-axis robotic arm** using:
- An **ESP-32** microcontroller (the brain)
- **6 stepper motors** (the muscles)
- **A4988/DRV8825 drivers** (the motor drivers)

Think of it like building your own mini industrial robot arm - the kind you see in factories picking up car parts, but desktop-sized and way more fun!

## Features

- **6 Independent Axes** - Base, shoulder, elbow, wrist pitch, wrist roll, and gripper
- **Hardware-Accelerated** - Uses FastAccelStepper for buttery smooth 200,000 steps/sec
- **WiFi Control** - REST API + web UI for wireless operation
- **Serial Control** - Direct USB connection for debugging
- **G-code Commands** - Industry-standard command format (like 3D printers!)
- **Python CLI** - Control your arm from the command line
- **Safety First** - Position limits, emergency stop, enable/disable

## Quick Demo

```bash
# Connect to your arm
roboarm-cli --url http://roboarm.local status

# Enable the motors
roboarm-cli enable

# Wave hello! (move base and shoulder)
roboarm-cli move --j1 1000 --j2 500 --wait

# Do a little dance
roboarm-cli move --j1 -1000 --j3 300 --wait
roboarm-cli move --j1 1000 --j3 -300 --wait

# Disable when done
roboarm-cli disable
```

Or use the web interface at `http://roboarm.local`:

```
+----------------------------------+
|  ROBOARM CONTROLLER              |
+----------------------------------+
|  [Enable]  [Disable]  [E-STOP]   |
|                                  |
|  Command: [G0 J1:1000    ] [Send]|
|                                  |
|  Status:                         |
|  J1: 0     J2: 0     J3: 0      |
|  J4: 0     J5: 0     J6: 0      |
+----------------------------------+
```

## Hardware Requirements

| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP-32 DevKit | 1 | Any ESP-WROOM-32 board works |
| A4988 or DRV8825 | 6 | Stepper motor drivers |
| NEMA 17 Stepper Motors | 6 | 1.8° step angle (200 steps/rev) |
| 12V/24V Power Supply | 1 | Match your motor voltage |
| Robot Arm Kit | 1 | 3D printed or aluminum frame |

### Wiring Overview

```
ESP-32                  A4988 Driver           Motor
┌──────┐               ┌──────────┐           ┌─────┐
│ GPIO │───── STEP ────│          │           │     │
│  16  │               │          │─── 1A ────│ A+  │
│      │               │          │─── 1B ────│ A-  │
│ GPIO │───── DIR ─────│          │─── 2A ────│ B+  │
│  17  │               │          │─── 2B ────│ B-  │
│      │               │          │           │     │
│ GPIO │───── EN ──────│          │           └─────┘
│  4   │               │          │
│      │               │  VMOT ───┼─── 12V/24V
│ GND  │───────────────│  GND  ───┼─── GND
│ 3.3V │───────────────│  VDD     │
└──────┘               └──────────┘
```

See [docs/wiring.md](docs/wiring.md) for the complete wiring guide.

## Installation

### 1. Flash the Firmware

```bash
# Install PlatformIO
pip install platformio

# Clone this repo
git clone https://github.com/banton/roboarm.git
cd roboarm

# Edit WiFi credentials
nano firmware/src/config.h
# Change WIFI_SSID and WIFI_PASSWORD

# Build and upload
cd firmware
pio run -t upload

# Open serial monitor to see output
pio device monitor
```

### 2. Install Python Tools (Optional)

```bash
cd host
pip install -e .

# Test the connection
roboarm-cli --url http://roboarm.local status
```

## Command Reference

### G-codes (Movement)

| Command | Description | Example |
|---------|-------------|---------|
| `G0` | Move to absolute position | `G0 J1:1000 J2:500` |
| `G1` | Move relative to current | `G1 J1:100 J3:-50` |
| `G28` | Home all axes (set to zero) | `G28` |

### M-codes (Control)

| Command | Description | Example |
|---------|-------------|---------|
| `M17` | Enable all motors | `M17` |
| `M18` | Disable all motors | `M18` |
| `M112` | **EMERGENCY STOP** | `M112` |
| `M114` | Report current positions | `M114` |
| `M503` | Report settings | `M503` |
| `?` | Quick status | `?` |

### Joint Naming

| Joint | Name | Range | Description |
|-------|------|-------|-------------|
| J1 | Base | ±100,000 steps | Rotates the whole arm |
| J2 | Shoulder | ±50,000 steps | First arm segment up/down |
| J3 | Elbow | ±50,000 steps | Second arm segment |
| J4 | Wrist Pitch | ±25,000 steps | Wrist tilts up/down |
| J5 | Wrist Roll | ±25,000 steps | Wrist rotates |
| J6 | Gripper | ±10,000 steps | Opens/closes gripper |

## REST API

Control your arm over HTTP:

```bash
# Get status
curl http://roboarm.local/api/status

# Enable motors
curl -X POST http://roboarm.local/api/enable \
  -H "Content-Type: application/json" \
  -d '{"enabled": true}'

# Move joints
curl -X POST http://roboarm.local/api/move \
  -H "Content-Type: application/json" \
  -d '{"j1": 1000, "j2": 500}'

# Send any G-code command
curl -X POST http://roboarm.local/api/command \
  -H "Content-Type: application/json" \
  -d '{"command": "G0 J1:2000 J2:1000 J3:500"}'
```

See [docs/api.md](docs/api.md) for complete API documentation.

## Project Structure

```
roboarm/
├── firmware/                # ESP-32 code (PlatformIO)
│   ├── src/
│   │   ├── main.cpp         # Entry point
│   │   ├── config.h         # Pin mappings & settings
│   │   ├── motor_controller # FastAccelStepper wrapper
│   │   ├── command_parser   # G-code parsing
│   │   └── web_server       # REST API & web UI
│   └── platformio.ini
│
├── host/                    # Python tools
│   ├── src/roboarm/
│   │   ├── client.py        # HTTP/Serial client
│   │   └── cli.py           # Command-line interface
│   └── pyproject.toml
│
└── docs/                    # Documentation
    ├── wiring.md            # Hardware connections
    └── api.md               # REST API reference
```

## Configuration

Edit `firmware/src/config.h` to customize:

```cpp
// WiFi credentials
#define WIFI_SSID "YourNetwork"
#define WIFI_PASSWORD "YourPassword"

// Motor pins (if your wiring is different)
const MotorConfig MOTOR_CONFIGS[MOTOR_COUNT] = {
    {16, 17, 4, 200, 16, 50000, 10000, false, "J1-Base"},
    // ... more motors
};

// Position limits (steps)
const int32_t POSITION_LIMITS_MIN[6] = {-100000, -50000, ...};
const int32_t POSITION_LIMITS_MAX[6] = { 100000,  50000, ...};
```

## Fun Project Ideas

Once you have your arm working, try these:

1. **Pick and Place** - Sort colored objects into bins
2. **Drawing Bot** - Attach a pen and draw pictures
3. **Laser Pointer Cat Toy** - Mount a laser and control it remotely
4. **Chess Player** - Move chess pieces on a board
5. **Bartender** - Pour drinks (carefully!)
6. **Plant Waterer** - Water your plants automatically
7. **Camera Mount** - Smooth panning for video
8. **Sign Language** - Spell out messages with hand signs

## Troubleshooting

### Motor doesn't move
- Check `M17` was sent to enable motors
- Verify wiring matches `config.h` pin assignments
- Check power supply voltage and current

### Motor vibrates but doesn't turn
- Swap one motor coil pair (A+/A- or B+/B-)
- Check microstepping jumpers on driver

### ESP-32 keeps resetting
- Add capacitors to power supply
- Check for brown-out from motor current draw
- Ensure adequate power supply current (2A+ recommended)

### Can't connect via WiFi
- Verify WiFi credentials in `config.h`
- Check serial monitor for IP address
- Try `roboarm.local` or the IP directly

## Technical Details

### Why FastAccelStepper?

We use [FastAccelStepper](https://github.com/gin66/FastAccelStepper) instead of the common AccelStepper library because:

| Feature | AccelStepper | FastAccelStepper |
|---------|--------------|------------------|
| Max speed | ~20,000 steps/sec | **200,000 steps/sec** |
| Timing | Software (blocking) | Hardware (MCPWM/RMT) |
| CPU usage | High | **Minimal** |
| Multi-motor | Sequential | **Near-simultaneous** |

This means smoother motion, faster speeds, and the ESP-32 can handle WiFi + 6 motors without breaking a sweat.

### ESP-32 Pin Selection

Not all ESP-32 pins are created equal! We carefully selected pins that:
- Are NOT connected to internal flash (GPIO 6-11)
- Are NOT input-only (GPIO 34-39)
- Don't affect boot behavior (strapping pins)

Safe pins used: 4, 16-19, 21-23, 25-27, 32-33

## Contributing

Contributions welcome! Feel free to:
- Report bugs
- Suggest features
- Submit pull requests
- Share your arm builds!

## License

MIT License - do whatever you want with it!

## Acknowledgments

- [FastAccelStepper](https://github.com/gin66/FastAccelStepper) - The magic behind smooth motion
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) - Non-blocking web server
- The ESP-32 community for excellent documentation

---

**Happy Building!**

If you build something cool, share it! Open an issue with photos/videos of your creation.

```
    _____
   /     \
  | () () |
   \  ^  /
    |||||
    |||||

  "I for one welcome
   our new robot
   overlords"
```
