"""
Roboarm - Python client for 6-axis robotic arm controller

Usage:
    from roboarm import RoboarmClient

    # HTTP client
    client = RoboarmClient("http://192.168.1.100")

    # Serial client
    client = RoboarmClient("serial:///dev/ttyUSB0")

    # Enable motors and move
    client.enable()
    client.move(j1=1000, j2=500)
    print(client.status())
    client.disable()
"""

from .client import RoboarmClient

__version__ = "0.1.0"
__all__ = ["RoboarmClient"]
