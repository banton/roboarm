# Roboarm Python Tools

Python CLI and library for controlling your Roboarm.

## Installation

```bash
pip install -e .
```

## Usage

```bash
# Check status
roboarm-cli --url http://roboarm.local status

# Enable motors
roboarm-cli enable

# Move joints
roboarm-cli move --j1 1000 --j2 500

# Send G-code
roboarm-cli gcode "G0 J1:2000 J3:1000"
```

## API

```python
from roboarm import RoboarmClient

client = RoboarmClient("http://roboarm.local")
status = await client.get_status()
await client.move(j1=1000, j2=500)
```
