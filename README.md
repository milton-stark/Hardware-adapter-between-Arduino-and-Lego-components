# Arduino ↔ LEGO Hardware Adapter Shield

A custom KiCad-designed shield that lets an Arduino Mega 2560 talk directly to LEGO MINDSTORMS NXT and EV3 sensors and motors, with demo firmware that exercises the board on a two-wheeled robot.

Studienarbeit, M.Sc. Mechatronics, University of Siegen. Supervised by Prof. Dr.-Ing. habil. Michael Gerke and Dipl.-Ing. Peter Sahm. Presented November 2025.

---

## About the project

LEGO MINDSTORMS hardware is well suited to teaching and prototyping: the sensors are robust, the motors have integrated encoders, and the mechanical system can be rebuilt in minutes. The limitation is the controller. The NXT and EV3 bricks run a closed environment, so anything beyond their intended programming model (a custom control loop, an unusual sensor, a different communication protocol) is difficult or impossible.

An Arduino has the opposite profile: fully open, but no way to plug LEGO components in. The connector is a proprietary RJ12 variant with an offset latch, the supply and signal lines are not arranged like standard Arduino headers, and the sensors do not share one protocol: some are analog, some use I²C, and the EV3 generation added UART.

This project bridges the two. A shield PCB, designed in KiCad, carries LEGO-compatible ports and routes each one to the correct Arduino pins with the necessary pull-ups and level handling. The result is that LEGO mechanics and sensors can be driven by arbitrary Arduino code.

## Why it's useful

- **Keeps the mechanics, replaces the brain.** LEGO's rapid rebuild advantage stays, but the control code is fully open and can be written in C++ with any library.
- **Handles all three sensor interfaces.** Analog, I²C, and UART LEGO sensors terminate on one board, so mixed sensor sets work without adapters per sensor.
- **A real PCB, not a breadboard.** Connector footprints, pull-ups, motor driver interface, and mounting holes matching the Mega form factor are laid out and manufacturable.
- **Reusable teaching platform.** Any Arduino control experiment (PID, state machines, custom sensor fusion) can be run on LEGO hardware that a lab already owns.

## What's on the board

| LEGO component | Interface | Notes |
|---|---|---|
| NXT/EV3 touch sensor | Digital input | Read with internal pull-up; pressed reads LOW |
| NXT light sensor | Analog | Separate line switches the sensor's own LED for reflected vs. ambient readings |
| EV3 light sensor | Analog | Same electrical class as the NXT sensor, different pinout |
| EV3 gyro sensor | UART | EV3 generation moved to serial framing rather than I²C |
| Ultrasonic sensor | I²C | Shares the bus with other I²C devices |
| BNO055 IMU (non-LEGO) | I²C | 9-axis absolute orientation, used as the pitch source |
| LEGO motors | PWM via L298N | Two channels, direction and enable per channel |

A 10 kΩ pull-up is required on the NXT light sensor line; the wiring table for that connector is documented in the report.

## Demo firmware

`Full_code.ino` is a working example that exercises every interface on the board at once. It runs on a two-wheeled LEGO chassis:

1. **Calibrate.** On boot the sketch blocks until the BNO055 reports full gyro calibration, printing the calibration status each half second. It then captures the current pitch as the reference.
2. **Wait for a command.** The robot stays still until `start` is typed into the serial monitor. `stop` disables the motors again.
3. **Run the loop.** Pitch is read from the IMU, the error against the reference pitch is fed through a PID controller, and the resulting output sets the PWM drive level for both motors. Pressing the touch sensor reverses direction.
4. **Log.** Pitch and commanded PWM are printed as a comma-separated pair, so the run can be watched live in the Arduino Serial Plotter.

Gains are set at the top of the file (`Kp`, `Ki`, `Kd`) along with the motor and sensor pin assignments.

### Serial commands

| Command | Effect |
|---|---|
| `start` | Enable the motors |
| `stop` | Disable the motors and set speed to zero |

## Repository contents

| File / folder | What it is |
|---|---|
| `Full_code.ino` | Demo firmware: IMU calibration, PID loop, motor drive, sensor reads, serial logging |
| `KiCAD files.zip` | Full KiCad project: schematic, PCB layout, and netlist |
| `PCB Schematic.pdf` | Schematic, viewable without installing KiCad |
| `PCB design.pdf` | Board layout and copper artwork |
| `Arduino_MountingHole.pretty/` | Custom KiCad footprint library for the Arduino Mega mounting holes and header spacing |
| `LEGO MINDSTORMS EV3 Hardware Developer Kit/` | LEGO's official hardware reference: connector pinouts, electrical characteristics, and sensor protocols |
| `Arbeit Report.pdf` | Full Studienarbeit report: design decisions, protocol analysis, and results |

## Getting started

**Build the board.** Unzip `KiCAD files.zip` and open the project in KiCad 7 or later. Export Gerbers from the PCB editor and send them to a fabricator. `PCB Schematic.pdf` and `PCB design.pdf` are there if you only want to read the design.

**Flash the firmware.** Install the Arduino IDE and add these libraries through the Library Manager:

```
Adafruit BNO055
Adafruit Unified Sensor
```

`Wire` ships with the IDE. Select **Arduino Mega 2560** as the board, open `Full_code.ino`, and upload.

**Run it.** Open the Serial Monitor at 9600 baud, hold the robot still until calibration completes, then type `start`.

## Known limitations

- The loop runs on a fixed 300 ms delay plus the light sensor's own settle delays, which caps the control rate at roughly 2 Hz. That is enough to demonstrate the interfaces, but too slow for true balance control.
- Both motors receive the same drive command, so the PID output modulates forward speed rather than correcting tilt differentially.
- The light sensor's reflected and ambient values are read but not yet used in the control logic.
- The integral term is uncapped, so it can wind up during a long tilt.

## Report

Design rationale, connector protocol analysis, and results are in [`Arbeit Report.pdf`](Arbeit%20Report.pdf).

## Built with

Arduino Mega 2560 · KiCad · Adafruit BNO055 · L298N motor driver · LEGO MINDSTORMS NXT/EV3
