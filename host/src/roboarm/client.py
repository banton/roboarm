"""
Roboarm Client - Communication with ESP-32 controller

Supports both HTTP (WiFi) and Serial (USB) connections.
"""

from __future__ import annotations

import json
import time
from dataclasses import dataclass
from typing import Any
from urllib.parse import urlparse

import httpx
import serial


@dataclass
class RoboarmStatus:
    """Current status of the robotic arm."""

    enabled: bool
    moving: bool
    positions: dict[str, int]
    targets: dict[str, int]
    distances: dict[str, int]
    ip: str | None = None
    uptime: int | None = None

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> RoboarmStatus:
        return cls(
            enabled=data.get("enabled", False),
            moving=data.get("moving", False),
            positions=data.get("positions", {}),
            targets=data.get("targets", {}),
            distances=data.get("distances", {}),
            ip=data.get("ip"),
            uptime=data.get("uptime"),
        )


class RoboarmClient:
    """
    Client for communicating with Roboarm ESP-32 controller.

    Supports HTTP (WiFi) and Serial (USB) connections.

    Args:
        url: Connection URL
            - HTTP: "http://192.168.1.100" or "http://roboarm.local"
            - Serial: "serial:///dev/ttyUSB0" or "serial://COM3"
        timeout: Connection timeout in seconds
        baud_rate: Serial baud rate (default: 115200)
    """

    def __init__(
        self,
        url: str,
        timeout: float = 10.0,
        baud_rate: int = 115200,
    ) -> None:
        self._url = url
        self._timeout = timeout
        self._baud_rate = baud_rate
        self._serial: serial.Serial | None = None
        self._http_client: httpx.Client | None = None

        # Determine connection type
        parsed = urlparse(url)
        if parsed.scheme == "serial":
            self._mode = "serial"
            self._serial_port = parsed.path or parsed.netloc
        elif parsed.scheme in ("http", "https"):
            self._mode = "http"
            self._base_url = url.rstrip("/")
        else:
            raise ValueError(f"Unsupported URL scheme: {parsed.scheme}")

    def connect(self) -> None:
        """Establish connection to the controller."""
        if self._mode == "serial":
            self._serial = serial.Serial(
                port=self._serial_port,
                baudrate=self._baud_rate,
                timeout=self._timeout,
            )
            # Wait for ESP-32 to be ready
            time.sleep(2)
            # Clear any startup messages
            self._serial.reset_input_buffer()
        else:
            self._http_client = httpx.Client(timeout=self._timeout)

    def disconnect(self) -> None:
        """Close connection to the controller."""
        if self._serial:
            self._serial.close()
            self._serial = None
        if self._http_client:
            self._http_client.close()
            self._http_client = None

    def __enter__(self) -> RoboarmClient:
        self.connect()
        return self

    def __exit__(self, *args: Any) -> None:
        self.disconnect()

    def send_command(self, command: str) -> dict[str, Any]:
        """
        Send a G-code command to the controller.

        Args:
            command: G-code command (e.g., "G0 J1:1000")

        Returns:
            Response dict with 'success' and 'message' keys
        """
        if self._mode == "serial":
            return self._send_serial(command)
        else:
            return self._send_http(command)

    def _send_serial(self, command: str) -> dict[str, Any]:
        if not self._serial:
            raise RuntimeError("Not connected. Call connect() first.")

        # Send command
        self._serial.write(f"{command}\n".encode())
        self._serial.flush()

        # Read response (until we get a line)
        response_lines = []
        timeout_end = time.time() + self._timeout

        while time.time() < timeout_end:
            if self._serial.in_waiting:
                line = self._serial.readline().decode().strip()
                if line:
                    response_lines.append(line)
                    # Check if we've received a complete response
                    if line.startswith("ok") or line.startswith("error:"):
                        break
            else:
                time.sleep(0.01)

        if not response_lines:
            return {"success": False, "message": "No response from controller"}

        # Parse response
        full_response = "\n".join(response_lines)
        if response_lines[-1].startswith("error:"):
            return {"success": False, "message": full_response}

        return {"success": True, "message": full_response}

    def _send_http(self, command: str) -> dict[str, Any]:
        if not self._http_client:
            raise RuntimeError("Not connected. Call connect() first.")

        response = self._http_client.post(
            f"{self._base_url}/api/command",
            json={"command": command},
        )
        return response.json()

    def status(self) -> RoboarmStatus:
        """Get current status of the robotic arm."""
        if self._mode == "serial":
            result = self.send_command("?")
            # Parse quick status format: "EM P:0,0,0,0,0,0"
            # This is a simplified implementation
            parts = result["message"].split()
            enabled = "E" in parts[0]
            moving = "M" in parts[0]

            positions = {}
            if len(parts) > 1 and parts[1].startswith("P:"):
                pos_values = parts[1][2:].split(",")
                for i, val in enumerate(pos_values):
                    positions[f"j{i + 1}"] = int(val)

            return RoboarmStatus(
                enabled=enabled,
                moving=moving,
                positions=positions,
                targets={},
                distances={},
            )
        else:
            if not self._http_client:
                raise RuntimeError("Not connected. Call connect() first.")

            response = self._http_client.get(f"{self._base_url}/api/status")
            return RoboarmStatus.from_dict(response.json())

    def enable(self) -> dict[str, Any]:
        """Enable all stepper motors."""
        return self.send_command("M17")

    def disable(self) -> dict[str, Any]:
        """Disable all stepper motors."""
        return self.send_command("M18")

    def emergency_stop(self) -> dict[str, Any]:
        """Emergency stop - immediately stop and disable all motors."""
        return self.send_command("M112")

    def move(
        self,
        j1: int | None = None,
        j2: int | None = None,
        j3: int | None = None,
        j4: int | None = None,
        j5: int | None = None,
        j6: int | None = None,
        relative: bool = False,
    ) -> dict[str, Any]:
        """
        Move joints to specified positions.

        Args:
            j1-j6: Target positions in steps (None to skip)
            relative: If True, positions are relative to current position

        Returns:
            Response dict
        """
        cmd = "G1" if relative else "G0"
        joints = [j1, j2, j3, j4, j5, j6]

        for i, pos in enumerate(joints, 1):
            if pos is not None:
                cmd += f" J{i}:{pos}"

        return self.send_command(cmd)

    def home(self) -> dict[str, Any]:
        """Home all axes (sets current position as zero)."""
        return self.send_command("G28")

    def get_positions(self) -> dict[str, int]:
        """Get current positions of all joints."""
        return self.status().positions

    def wait_for_idle(self, timeout: float = 60.0, poll_interval: float = 0.1) -> bool:
        """
        Wait for all motors to stop moving.

        Args:
            timeout: Maximum time to wait in seconds
            poll_interval: Time between status checks

        Returns:
            True if motors stopped, False if timeout
        """
        end_time = time.time() + timeout

        while time.time() < end_time:
            status = self.status()
            if not status.moving:
                return True
            time.sleep(poll_interval)

        return False
