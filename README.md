# Veda Embedded Device Control (VEDC)

## Overview

The **Veda Embedded Device Control (VEDC)** system is a Linux-based client–server application designed for controlling multiple embedded devices connected to a single board computer (e.g., Raspberry Pi).  
It provides a command-driven TCP protocol for remote operation of LEDs, buzzers, FND displays, and CDS light sensors.

**Key features:**
- Modular architecture with **dynamic loading** of device control libraries
- **Client–server** communication over TCP (default port: **60000**)
- **Multi-threaded** device operations with mutex protection
- Device control via **wiringPi**, **softPwm**, **softTone**, and **wiringPiI2C**

## Architecture

![Block Diagram](docs/block%20diagram.png)

**Main Components:**
1. **Client (CLI)**  
   - Sends commands in format `CMD:<id>[:param]` over TCP  
   - Example: `CMD:1` (LED ON), `CMD:6:5` (FND countdown from 5)  

2. **Server Core** (`main.c`, `server.c`)  
   - Initializes resources, loads device libraries, opens socket, and handles incoming connections  

3. **Handler** (`handler.c`)  
   - Parses commands, validates parameters, and dispatches device actions to worker threads  

4. **Device Manager** (`device.c`)  
   - Initializes wiringPi, manages device lifecycle (init/control/cleanup)  

5. **Dynamic Loader**  
   - Loads `.so` device modules at runtime:  
     - `libled.so` — LED control (softPwm)  
     - `libbuzzer.so` — Buzzer control (softTone)  
     - `libcds.so` — CDS sensor readout (I2C + GPIO LED)  
     - `libfnd.so` — 7-segment display control  

6. **Device Libraries**  
   - Each runs in a dedicated thread for asynchronous operation  
   - Commands:  
     - CMD:1 — LED ON  
     - CMD:2 — LED OFF  
     - CMD:3 — BUZZER ON  
     - CMD:4 — BUZZER OFF  
     - CMD:5 — CDS READ  
     - CMD:6:x — FND COUNTDOWN (x = 0..9)

## Directory Structur
```bash
veda_embedded_device_control/
├── client
│   ├── bin
│   ├── build
│   ├── src
│   │   └── main.c
│   └── CMakeLists.txt
├── server
│   ├── bin
│   │   └── terminate_server.sh
│   ├── build
│   ├── include
│   │   ├── config.h
│   │   └── interface.h
│   ├── lib
│   │   ├── buzzer
│   │   │   ├── buzzer.c
│   │   │   └── Makefile
│   │   ├── cds
│   │   │   ├── cds.c
│   │   │   └── Makefile
│   │   ├── fnd
│   │   │   ├── fnd.c
│   │   │   └── Makefile
│   │   └── led
│   │       ├── led.c
│   │       └── Makefile
│   ├── src
│   │   ├── core
│   │   │   ├── main.c
│   │   │   ├── server.c
│   │   │   └── server.h
│   │   ├── device
│   │   │   ├── device.c
│   │   │   └── device.h
│   │   ├── handler
│   │   │   ├── handler.c
│   │   │   └── handler.h
│   │   └── socket
│   │       ├── socket.c
│   │       └── socket.h
│   └── CMakeLists.txt
└── README.md
```

## How to Build & Run

### 1. Build Device Libraries

Each device module must be built into a shared object (`.so`) before building the server:

```bash
cd server/lib/led && make
cd ../buzzer && make
cd ../cds && make
cd ../fnd && make
```

This will produce:
```bash
libled.so, libbuzzer.so, libcds.so, libfnd.so
```

Ensure these are placed in the path defined in config.h (default: server/lib/<device>/).

### 2. Build Client & Server

```bash
cd server
mkdir build && cd build
cmake ..
make
```

```bash
cd ../../client
mkdir build && cd build
cmake ..
make
```

### 3. Run the Server

```bash
./server_app &
```

Runs as a background daemon
Listens on port 60000

### 4. Run the Client

```bash
./client_app
```

Connects to the server and waits for user commands

## How to Test

Example commands from the client:

```bash
CMD:1        # LED ON
CMD:2        # LED OFF
CMD:3        # BUZZER ON
CMD:4        # BUZZER OFF
CMD:5        # CDS sensor read
CMD:6:7      # FND countdown from 7
```

Expected behavior:
- Device responds accordingly
- CDS returns a measured light value
- FND displays numbers down to 0

## Lessons Learned

Dynamic loading of device modules increases flexibility but requires careful path management
Thread-based device control improves responsiveness but needs proper mutex handling
WiringPi simplifies GPIO handling but is deprecated; future versions should consider libgpiod
Clearly documenting the command protocol is essential for client–server integration
Separation of device logic into independent .so modules aids maintainability and testing