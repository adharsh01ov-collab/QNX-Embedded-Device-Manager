# QNX Embedded Device Manager

A real-time embedded device monitoring and control system built on **QNX Neutrino RTOS** using **QNX native IPC**.

The project demonstrates inter-process communication between a server and multiple clients, device-state management, real-time monitoring, mutex-based synchronization, and structured request/response messaging.

---

## Table of Contents

- [Overview](#overview)
- [Technologies Used](#technologies-used)
- [Repository Structure](#repository-structure)
- [System Architecture](#system-architecture)
- [IPC Communication](#ipc-communication)
- [Supported Commands](#supported-commands)
- [Device Parameters](#device-parameters)
- [Build and Run](#build-and-run)
- [Example Output](#example-output)
- [Embedded Concepts Demonstrated](#embedded-concepts-demonstrated)
- [Possible Future Improvements](#possible-future-improvements)
- [Resume Description](#resume-description)
- [Author](#author)

---

## Overview

The **QNX Embedded Device Manager** simulates an embedded device controller running on QNX. It consists of three programs:

| Program | File | Role |
| --- | --- | --- |
| **Device Manager Server** | `server.c` | Maintains device state, processes client commands, updates temperature and RPM, controls speed, and tracks communication statistics |
| **Interactive Client** | `client.c` | Menu-driven client to start/stop the device, change speed, fetch status and statistics, reset the device, and shut down the server |
| **Monitoring Client** | `monitor_client.c` | Periodically requests data from the server and displays live device values |

---

## Technologies Used

- C
- QNX Neutrino RTOS
- QNX Momentics IDE
- VMware Workstation
- POSIX Threads (`pthread`)
- QNX native IPC: `name_attach()`, `name_open()`, `MsgSend()`, `MsgReceive()`, `MsgReply()`
- Mutex synchronization
- Multithreading

---

## Repository Structure

```text
.
├── server.c            # Device Manager server
├── client.c            # Interactive menu client
├── monitor_client.c    # Automatic live monitoring client
└── README.md
```

---

## System Architecture

```text
             +----------------------+
             |   Monitoring Client  |
             |   (Live Data Fetch)  |
             +----------+-----------+
                        |
                        | MsgSend()
                        v
             +----------------------+
             |                      |
             |  QNX Device Manager  |
             |       Server         |
             |                      |
             +----------+-----------+
                        ^
                        | MsgSend()
                        |
             +----------+-----------+
             |  Interactive Client  |
             +----------------------+

                    QNX VM
```

---

## IPC Communication

The server registers a QNX named service:

```c
name_attach(NULL, "qnx_device_manager", 0);
```

Clients connect with:

```c
name_open("qnx_device_manager", 0);
```

Clients send requests with `MsgSend()`. The server receives them with `MsgReceive()` and responds with `MsgReply()`, giving synchronous request/response communication between processes.

### Message Structures

```c
typedef struct
{
    int command;
    int value;
} request_t;

typedef struct
{
    int status;
    int temperature;
    int speed;
    int rpm;
    int device_state;
    int messages_received;
    char message[128];
} response_t;
```

---

## Supported Commands

| Command | Description |
| --- | --- |
| `CMD_GET_STATUS` | Fetch current device status |
| `CMD_SET_SPEED` | Change device speed (0-200) |
| `CMD_GET_STATS` | Fetch communication statistics |
| `CMD_START` | Start device |
| `CMD_STOP` | Stop device |
| `CMD_RESET` | Reset device |
| `CMD_SHUTDOWN` | Shut down server |

---

## Device Parameters

The server maintains:

- Temperature
- Speed
- RPM
- Device state (`STOPPED` / `RUNNING`)
- Messages received

Simulated RPM is calculated as:

```text
RPM = Speed x 40
```

For example, `Speed = 100` gives `RPM = 4000`.

While the device is `RUNNING`, a background monitor thread increases the temperature by 1 C every 2 seconds, wrapping back to 25 C after 100 C.

---

## Build and Run

### 1. Start the QNX VM

Start the QNX VM in VMware Workstation and make sure the system is running.

### 2. Build

**Using QNX Momentics:** create a project for each source file and run `Project -> Build Project`.

**Using the command line (QNX SDP):**

```bash
qcc -o server         server.c
qcc -o client         client.c
qcc -o monitor_client monitor_client.c
```

### 3. Run the server

```bash
./server
```

Expected output:

```text
Server started successfully.
Service Name: qnx_device_manager
```

### 4. Run the interactive client (new terminal)

```bash
./client
```

Expected output:

```text
Connected to QNX Device Manager.
```

### 5. Run the monitoring client (another terminal)

```bash
./monitor_client
```

It refreshes the live device values every two seconds.

> The server must be running before any client is started, and all programs must run on the same QNX target.

---

## Example Output

### Server

```text
=====================================
 QNX Embedded Device Manager Server
=====================================
Server started successfully.
Service Name: qnx_device_manager
```

### Interactive Client

```text
Connected to QNX Device Manager.

========== MENU ==========
1. Get Status
2. Set Speed
3. Get Statistics
4. Start Device
5. Stop Device
6. Reset Device
7. Shutdown Server
8. Exit Client
==========================

Enter choice: 4

Server Response: Device started successfully
```

```text
Enter choice: 2
Enter speed (0-200): 100

Server Response: Speed updated successfully
```

```text
Enter choice: 1

========== DEVICE STATUS ==========
Temperature : 28 C
Speed       : 100
RPM         : 4000
State       : RUNNING
Messages    : 3
Message     : Status retrieved successfully
===================================
```

### Monitoring Client

```text
=====================================
       QNX DEVICE MONITOR
=====================================
Temperature : 32 C
Speed       : 100
RPM         : 4000
Device State: RUNNING
Messages    : 15
=====================================
```

---

## Project Workflow

```text
Server starts
      |
Registers QNX named service
      |
Client connects using name_open()
      |
Client builds request  ->  MsgSend()
      |
Server MsgReceive()  ->  processes command  ->  updates device state
      |
Server MsgReply()
      |
Client receives response and displays data
```

---

## Output
--SERVER--
<img width="1920" height="1080" alt="Screenshot 2026-10-07 234246" src="https://github.com/user-attachments/assets/3ba9d0c1-a74a-4fc9-863a-43e9efcfded0" />

--CLIENT--
<img width="1920" height="1080" alt="Screenshot 2026-10-07 234657" src="https://github.com/user-attachments/assets/a89b8336-4892-414e-90f8-52bb6c847681" />

--RUNNING--

https://github.com/user-attachments/assets/4e905d07-dc2e-4be4-8e46-4f5f4107194f

--------


## Embedded Concepts Demonstrated

1. **Inter-process communication**: QNX native message passing (`MsgSend` / `MsgReceive` / `MsgReply`) between independent processes.
2. **Named services**: `name_attach()` and `name_open()` for service discovery.
3. **Multithreading**: a monitor thread created with `pthread_create()` updates simulated device parameters.
4. **Synchronization**: a mutex protects shared device state between the IPC thread and the monitor thread.
5. **Real-time monitoring**: periodic updates of temperature, RPM, and device state simulate a continuously running device.
6. **Structured communication**: typed request/response structs instead of plain strings, similar to real embedded control and status protocols.

---

## Possible Future Improvements

- CAN bus simulation
- TCP/IP and UART communication
- Sensor simulation and real sensor integration
- Error logging, watchdog timer, and fault detection
- Multiple-client handling and priority-based IPC
- Shared memory and message queues
- QNX resource manager
- Hardware GPIO control
- SQLite/database logging
- Network-based monitoring dashboard

---

## Resume Description

**QNX Embedded Device Manager | C, QNX Neutrino RTOS, IPC**

Developed a multithreaded embedded device management system on QNX Neutrino RTOS using native message-passing IPC. Implemented named services, synchronous request/response communication, device-state management, real-time parameter monitoring, mutex synchronization, structured IPC messages, and automated client-side status monitoring.

**Key skills:** C, QNX Neutrino RTOS, QNX Momentics, Native IPC, POSIX Threads, Mutex Synchronization, Multithreading, Embedded Systems, Real-Time Monitoring, VMware

---

## Author

**Adharsh V**
B.E. Electronics and Communication Engineering, Saveetha Engineering College

Developed as an embedded systems learning and portfolio project using QNX Neutrino RTOS and QNX Momentics.
