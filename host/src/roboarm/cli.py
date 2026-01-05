"""
Roboarm CLI - Command-line interface for robotic arm control.

Usage:
    roboarm-cli status --url http://192.168.1.100
    roboarm-cli move --j1 1000 --j2 500 --url http://192.168.1.100
    roboarm-cli enable --url serial:///dev/ttyUSB0
"""

from __future__ import annotations

from typing import Annotated

import typer
from rich import print as rprint
from rich.console import Console
from rich.table import Table

from .client import RoboarmClient

app = typer.Typer(
    name="roboarm-cli",
    help="Command-line interface for Roboarm 6-axis robotic arm controller",
    no_args_is_help=True,
)
console = Console()


def get_client(url: str) -> RoboarmClient:
    """Create and connect a client."""
    client = RoboarmClient(url)
    client.connect()
    return client


@app.command()
def status(
    url: Annotated[str, typer.Option("--url", "-u", help="Controller URL")] = "http://roboarm.local",
) -> None:
    """Get current status of the robotic arm."""
    with get_client(url) as client:
        status = client.status()

        table = Table(title="Roboarm Status")
        table.add_column("Property", style="cyan")
        table.add_column("Value", style="green")

        table.add_row("Enabled", "Yes" if status.enabled else "No")
        table.add_row("Moving", "Yes" if status.moving else "No")

        if status.ip:
            table.add_row("IP Address", status.ip)
        if status.uptime:
            table.add_row("Uptime", f"{status.uptime}s")

        console.print(table)

        # Position table
        pos_table = Table(title="Joint Positions")
        pos_table.add_column("Joint", style="cyan")
        pos_table.add_column("Position", style="green")
        pos_table.add_column("Target", style="yellow")
        pos_table.add_column("Distance", style="magenta")

        for i in range(1, 7):
            key = f"j{i}"
            pos_table.add_row(
                f"J{i}",
                str(status.positions.get(key, "?")),
                str(status.targets.get(key, "?")),
                str(status.distances.get(key, "?")),
            )

        console.print(pos_table)


@app.command()
def enable(
    url: Annotated[str, typer.Option("--url", "-u", help="Controller URL")] = "http://roboarm.local",
) -> None:
    """Enable all stepper motors."""
    with get_client(url) as client:
        result = client.enable()
        if result["success"]:
            rprint("[green]Motors enabled[/green]")
        else:
            rprint(f"[red]Error: {result['message']}[/red]")


@app.command()
def disable(
    url: Annotated[str, typer.Option("--url", "-u", help="Controller URL")] = "http://roboarm.local",
) -> None:
    """Disable all stepper motors."""
    with get_client(url) as client:
        result = client.disable()
        if result["success"]:
            rprint("[yellow]Motors disabled[/yellow]")
        else:
            rprint(f"[red]Error: {result['message']}[/red]")


@app.command()
def stop(
    url: Annotated[str, typer.Option("--url", "-u", help="Controller URL")] = "http://roboarm.local",
) -> None:
    """Emergency stop - immediately stop and disable all motors."""
    with get_client(url) as client:
        result = client.emergency_stop()
        rprint("[red bold]EMERGENCY STOP[/red bold]")
        rprint(result["message"])


@app.command()
def move(
    url: Annotated[str, typer.Option("--url", "-u", help="Controller URL")] = "http://roboarm.local",
    j1: Annotated[int | None, typer.Option("--j1", help="Joint 1 position")] = None,
    j2: Annotated[int | None, typer.Option("--j2", help="Joint 2 position")] = None,
    j3: Annotated[int | None, typer.Option("--j3", help="Joint 3 position")] = None,
    j4: Annotated[int | None, typer.Option("--j4", help="Joint 4 position")] = None,
    j5: Annotated[int | None, typer.Option("--j5", help="Joint 5 position")] = None,
    j6: Annotated[int | None, typer.Option("--j6", help="Joint 6 position")] = None,
    relative: Annotated[bool, typer.Option("--relative", "-r", help="Relative move")] = False,
    wait: Annotated[bool, typer.Option("--wait", "-w", help="Wait for completion")] = False,
) -> None:
    """Move joints to specified positions."""
    if all(j is None for j in [j1, j2, j3, j4, j5, j6]):
        rprint("[red]Error: Specify at least one joint position (--j1, --j2, etc.)[/red]")
        raise typer.Exit(1)

    with get_client(url) as client:
        result = client.move(j1=j1, j2=j2, j3=j3, j4=j4, j5=j5, j6=j6, relative=relative)

        if result["success"]:
            mode = "relative" if relative else "absolute"
            rprint(f"[green]Move command sent ({mode})[/green]")

            if wait:
                rprint("Waiting for move to complete...")
                if client.wait_for_idle():
                    rprint("[green]Move complete[/green]")
                else:
                    rprint("[yellow]Timeout waiting for move[/yellow]")
        else:
            rprint(f"[red]Error: {result['message']}[/red]")


@app.command()
def home(
    url: Annotated[str, typer.Option("--url", "-u", help="Controller URL")] = "http://roboarm.local",
) -> None:
    """Home all axes (sets current position as zero)."""
    with get_client(url) as client:
        result = client.home()
        if result["success"]:
            rprint("[green]All joints homed (zeroed)[/green]")
        else:
            rprint(f"[red]Error: {result['message']}[/red]")


@app.command()
def send(
    command: Annotated[str, typer.Argument(help="G-code command to send")],
    url: Annotated[str, typer.Option("--url", "-u", help="Controller URL")] = "http://roboarm.local",
) -> None:
    """Send a raw G-code command."""
    with get_client(url) as client:
        result = client.send_command(command)
        if result["success"]:
            rprint(f"[green]{result['message']}[/green]")
        else:
            rprint(f"[red]{result['message']}[/red]")


if __name__ == "__main__":
    app()
