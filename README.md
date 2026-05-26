# Modular ESP32 MQTT Firmware
### ESP-IDF + FreeRTOS Telemetry System

Modular embedded firmware developed using ESP-IDF and FreeRTOS implementing MQTT-based telemetry communication, queue-driven task decoupling, WiFi connectivity, persistent configuration storage, and system health monitoring.

## Key Features

- Modular ESP-IDF firmware architecture
- FreeRTOS task-based design
- Queue-based producer-consumer telemetry pipeline
- MQTT publish/subscribe communication
- WiFi reconnect handling
- EventGroup synchronization
- Persistent configuration using NVS
- Heap and RSSI telemetry monitoring
- Real-time JSON telemetry streaming


## System Architecture

Telemetry Task
→ FreeRTOS Queue
→ MQTT Publish Task
→ MQTT Broker
→ Node-RED Dashboard


## Firmware Architecture

### app_main.c

Responsible for:
- NVS initialization
- WiFi initialization
- MQTT startup sequencing
- Telemetry startup

Waits for WiFi connection before MQTT initialization.

---

### wifi_manager.c / wifi_manager.h

Responsibilities:
- WiFi station initialization
- WiFi event handling
- Reconnect handling
- EventGroup synchronization

---

### mqtt_manager.c / mqtt_manager.h

Responsibilities:
- MQTT client initialization
- MQTT event callbacks
- MQTT publish task
- Queue consumer handling
- Topic subscription handling

---

### telemetry.c / telemetry.h

Responsibilities:
- Telemetry generation
- Queue producer handling
- JSON payload creation
- Heap monitoring
- RSSI monitoring
- Reliability statistics
- NVS persistent interval storage
## System Flow

```text
Power On
→ Bootloader
→ app_main()
→ NVS Initialization
→ WiFi Initialization
→ Wait for WIFI_CONNECTED_BIT
→ MQTT Initialization
→ MQTT Publish Task Start
→ Telemetry Task Start
→ Generate JSON Telemetry
→ Push Payload to FreeRTOS Queue
→ MQTT Publish Task Consumes Queue
→ MQTT Broker
→ Node-RED Dashboard
```
## Engineering Decisions

### Why Queue-Based Architecture?

Telemetry generation was decoupled from MQTT publishing to avoid blocking application logic during network instability and reconnect conditions.

---

### Why EventGroups?

EventGroups were used for synchronization between WiFi connection state and MQTT startup sequence.

---

### Why NVS?

NVS allows runtime configuration persistence across reboot cycles without recompilation.
## Debugging Challenges Solved

### 1. NVS Initialization Reboot Loop

Issue:
WiFi initialization started before NVS initialization causing reboot loops.

Fix:
Moved NVS initialization before WiFi startup sequence.

---

### 2. DNS getaddrinfo() Failure

Issue:
MQTT client started before valid IP assignment.

Fix:
Delayed MQTT initialization until WIFI_CONNECTED_BIT was set.

---

### 3. Queue Assertion Failure

Issue:
MQTT publish task started before queue creation.

Fix:
Corrected startup order between telemetry and MQTT initialization.

---

### 4. MQTT Topic Collision

Issue:
Default public topic caused payload conflicts.

Fix:
Implemented unique namespace-based MQTT topics.
## Repository Structure

```text
.
├── main/
│   ├── app_main.c
│   ├── wifi_manager.c
│   ├── wifi_manager.h
│   ├── mqtt_manager.c
│   ├── mqtt_manager.h
│   ├── telemetry.c
│   └── telemetry.h
├── screenshots/
├── diagrams/
└── README.md
```
## Tools & Technologies

### Embedded Platforms
- ESP32
- ESP-IDF v6.0

### RTOS
- FreeRTOS

### Communication Protocols
- MQTT
- UART
- SPI
- I2C

### Networking
- WiFi
- TCP/IP

### Development Tools
- Git
- GitHub
- Node-RED
- MQTT Explorer
- ESP-IDF Monitor
## Example Telemetry Payload

```json
{
  "device_id":"device_001",
  "seq":31,
  "uptime_ms":33370,
  "temp":27.5,
  "heap_free":205776,
  "heap_alert":0,
  "rssi":-52,
  "sent":30,
  "failed":0,
  "reconnect":0
}
```
## MQTT Topics

Broker:
```text
broker.hivemq.com
```

Telemetry Topic:
```text
edgepulse/manjunatha144/device_001/telemetry
```

Command Topic:
```text
edgepulse/manjunatha144/device_001/cmd
```
## Build & Run

### Clone Repository

```bash
git clone https://github.com/Manjunatha144/modular-esp32-mqtt-firmware.git
```

---

### Open ESP-IDF Environment

Configure ESP-IDF environment variables.

---

### Build Project

```bash
idf.py build
```

---

### Flash Firmware

```bash
idf.py -p PORT flash
```

---

### Open Serial Monitor

```bash
idf.py monitor
```
## Serial Monitor Output

![Serial Monitor](screenshots/esp32_serial_monitor_output.png)

## Firmware Architecture Diagram

![Architecture Diagram](diagrams/firmware_architecture.jpg)

## External Dashboard Integration

Telemetry data from this firmware was visualized using a Node-RED MQTT dashboard with real-time monitoring and command/control workflows.

Related Repository:
https://github.com/Manjunatha144/mqtt-telemetry-dashboard-system

## Author

### Manjunatha H

Embedded and IoT Firmware Developer


