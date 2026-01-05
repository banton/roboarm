# REST API Documentation

## Overview

The Roboarm controller exposes a REST API over WiFi for remote control.

**Base URL:** `http://<esp32-ip>/api/`

All endpoints accept and return JSON.

## Endpoints

### GET /api/status

Get current status of the robotic arm.

**Response:**
```json
{
  "enabled": true,
  "moving": false,
  "positions": {
    "j1": 0,
    "j2": 0,
    "j3": 0,
    "j4": 0,
    "j5": 0,
    "j6": 0
  },
  "targets": {
    "j1": 0,
    "j2": 0,
    "j3": 0,
    "j4": 0,
    "j5": 0,
    "j6": 0
  },
  "distances": {
    "j1": 0,
    "j2": 0,
    "j3": 0,
    "j4": 0,
    "j5": 0,
    "j6": 0
  },
  "ip": "192.168.1.100",
  "uptime": 12345
}
```

### POST /api/command

Execute a G-code command.

**Request:**
```json
{
  "command": "G0 J1:1000 J2:500"
}
```

**Response:**
```json
{
  "success": true,
  "message": "ok"
}
```

**Error Response:**
```json
{
  "success": false,
  "message": "error: Motors disabled"
}
```

### POST /api/move

Move joints to specified positions (alternative to G-code).

**Request:**
```json
{
  "j1": 1000,
  "j2": 500
}
```

Only include joints you want to move. Omitted joints are not affected.

**Response:**
```json
{
  "success": true,
  "message": "ok",
  "command": "G0 J1:1000 J2:500"
}
```

### POST /api/enable

Enable or disable stepper motors.

**Request:**
```json
{
  "enabled": true
}
```

**Response:**
```json
{
  "success": true,
  "enabled": true
}
```

### GET /api/config

Get motor configuration.

**Response:**
```json
{
  "motor_count": 6,
  "enable_pin": 15,
  "motors": [
    {
      "joint": 1,
      "name": "J1-Base",
      "step_pin": 2,
      "dir_pin": 4,
      "steps_per_rev": 200,
      "max_speed": 1000,
      "acceleration": 500,
      "invert_dir": false
    },
    ...
  ]
}
```

## G-code Commands

Send these via the `/api/command` endpoint:

| Command | Description | Example |
|---------|-------------|---------|
| `G0` | Move to absolute position | `G0 J1:1000 J2:500` |
| `G1` | Move relative | `G1 J1:100` |
| `G28` | Home (set current as zero) | `G28` |
| `M17` | Enable motors | `M17` |
| `M18` | Disable motors | `M18` |
| `M112` | Emergency stop | `M112` |
| `M114` | Report positions | `M114` |
| `M503` | Report settings | `M503` |
| `?` | Quick status | `?` |

## Examples

### cURL

```bash
# Get status
curl http://192.168.1.100/api/status

# Enable motors
curl -X POST http://192.168.1.100/api/enable \
  -H "Content-Type: application/json" \
  -d '{"enabled": true}'

# Move joint 1 to position 1000
curl -X POST http://192.168.1.100/api/move \
  -H "Content-Type: application/json" \
  -d '{"j1": 1000}'

# Send G-code command
curl -X POST http://192.168.1.100/api/command \
  -H "Content-Type: application/json" \
  -d '{"command": "G0 J1:1000 J2:500"}'

# Emergency stop
curl -X POST http://192.168.1.100/api/command \
  -H "Content-Type: application/json" \
  -d '{"command": "M112"}'
```

### Python

```python
import httpx

BASE_URL = "http://192.168.1.100"

# Get status
response = httpx.get(f"{BASE_URL}/api/status")
print(response.json())

# Enable motors
httpx.post(f"{BASE_URL}/api/enable", json={"enabled": True})

# Move joints
httpx.post(f"{BASE_URL}/api/move", json={"j1": 1000, "j2": 500})
```

### JavaScript

```javascript
const BASE_URL = 'http://192.168.1.100';

// Get status
const status = await fetch(`${BASE_URL}/api/status`).then(r => r.json());

// Enable motors
await fetch(`${BASE_URL}/api/enable`, {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({ enabled: true })
});

// Move joints
await fetch(`${BASE_URL}/api/move`, {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({ j1: 1000, j2: 500 })
});
```

## Error Codes

| HTTP Status | Meaning |
|-------------|---------|
| 200 | Success |
| 400 | Bad request (invalid JSON or command) |
| 404 | Endpoint not found |
| 500 | Internal server error |

## CORS

All endpoints include CORS headers allowing requests from any origin.
