# Roboarm

A 6-axis robotic arm controller using ESP-32 with A4988/DRV8825 stepper motor drivers.

## Features

- 6 independent stepper motor axes (J1-J6)
- WiFi and Serial communication
- G-code style command interface
- REST API for remote control
- Python CLI for testing and automation

## Project Structure

```
roboarm/
├── firmware/          # ESP-32 PlatformIO project
├── host/              # Python host-side tools
└── docs/              # Documentation
```

## Quick Start

### Firmware

1. Install [PlatformIO](https://platformio.org/)
2. Open the `firmware/` directory in VS Code with PlatformIO extension
3. Configure WiFi credentials in `src/config.h`
4. Build and upload to ESP-32

```bash
cd firmware
pio run -t upload
```

### Host Tools

```bash
cd host
pip install -e .
roboarm-cli --help
```

## Commands

| Command | Description |
|---------|-------------|
| `M17` | Enable all steppers |
| `M18` | Disable all steppers |
| `M114` | Report current positions |
| `G0 J1:1000 J2:500` | Move joints to position |

## Hardware

- ESP-32 DevKit (ESP-WROOM-32)
- 6x A4988 or DRV8825 stepper drivers
- 6x Stepper motors (NEMA 17 recommended)
- 24V power supply (or 12V depending on motors)

See [docs/wiring.md](docs/wiring.md) for connection details.

## API

REST API available at `http://<esp32-ip>/api/`

See [docs/api.md](docs/api.md) for full documentation.

## License

MIT
