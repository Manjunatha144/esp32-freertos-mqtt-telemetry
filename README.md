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

```text
+----------------------+
|   Telemetry Task     |
|----------------------|
| Generate JSON Data   |
| Heap/RSSI Monitoring |
| Reliability Metrics  |
+----------+-----------+
           |
           v
+----------------------+
|   FreeRTOS Queue     |
+----------+-----------+
           |
           v
+----------------------+
|  MQTT Publish Task   |
|----------------------|
| Queue Consumer       |
| MQTT Publish Handler |
+----------+-----------+
           |
           v
+----------------------+
|   MQTT Broker        |
+----------+-----------+
           |
           v
+----------------------+
| Node-RED Dashboard   |
+----------------------+
```
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
