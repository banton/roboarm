# Roboarm Project Instructions

## Project Overview

This is a 6-axis robotic arm controller project with:
- **Firmware**: ESP-32 with PlatformIO (Arduino framework)
- **Host Tools**: Python CLI and library
- **Hardware**: A4988/DRV8825 stepper drivers

## Architecture

### Firmware Components
- `config.h` - Pin mappings and motor configuration
- `motor_controller` - AccelStepper wrapper for 6 axes
- `command_parser` - G-code style command parsing
- `web_server` - REST API and async web server
- `main.cpp` - WiFi setup, Serial handling, main loop

### Communication Protocol
G-code style commands over Serial (115200 baud) or HTTP:
- `G0 J1:1000` - Move joint 1 to position 1000 steps
- `M17` / `M18` - Enable/disable steppers
- `M114` - Report positions
- `M503` - Report settings

### Motor Axes
| Joint | Purpose | Default Pins (Step/Dir) |
|-------|---------|------------------------|
| J1 | Base rotation | 2/4 |
| J2 | Shoulder | 16/17 |
| J3 | Elbow | 18/19 |
| J4 | Wrist pitch | 21/22 |
| J5 | Wrist roll | 23/25 |
| J6 | End effector | 26/27 |

## Development Guidelines

### Firmware
- Use AccelStepper for motor control (non-blocking)
- All motor movements must be non-blocking for web server
- JSON responses for API endpoints
- Serial responses use `ok` / `error:` prefix

### Python Host
- Use `httpx` for async HTTP
- Use `pyserial` for serial communication
- CLI built with `typer`

## Testing

### Firmware Testing
```bash
cd firmware
pio test
```

### Python Testing
```bash
cd host
pytest
```

## Key Files

- `firmware/src/config.h` - Change pin mappings here
- `firmware/src/main.cpp` - WiFi credentials
- `host/src/roboarm/client.py` - Communication client
