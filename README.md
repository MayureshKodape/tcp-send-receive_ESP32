# ESP32 TCP Send/Receive (Client + Server)

A minimal TCP example demonstrating **reliable data transfer over Wi-Fi** using **TCP sockets**.

This repository includes:
- `TCP_server.cpp` → TCP Server running on ESP32 (listens on a port, receives data, responds/echoes)
- `client_TCP.cpp` → TCP Client (connects to ESP32 server and sends/receives data)

> Why TCP? TCP provides **ordered + reliable** delivery, useful for IoT control and industrial communication.

---

## How It Works (Flow)

1) ESP32 connects to Wi-Fi (STA mode)  
2) ESP32 starts a TCP server: `0.0.0.0:<PORT>`  
3) A client connects using: `<ESP32_IP>:<PORT>`  
4) Client sends data → ESP32 receives → ESP32 replies (echo/ack)  
5) Client prints received response

---

## Requirements
- ESP32 / ESP32-S3 board
- Wi-Fi network (2.4 GHz)
- ESP-IDF installed (recommended) OR your preferred build method
- A TCP client tool (one of these):
  - Your provided `client_TCP.cpp`
  - `nc` (netcat)
  - Python socket client

---

## Run ESP32 TCP Server (ESP-IDF Style)

> Run commands inside the project folder.

### 1) Set Target
```bash
idf.py set-target esp32
# OR
idf.py set-target esp32s3
2) Configure Wi-Fi + Port

If your project uses menuconfig:

idf.py menuconfig


Set:

Wi-Fi SSID / Password

Server Port (if available)

If your code uses hardcoded SSID/PASS/PORT in TCP_server.cpp, update them there.

3) Build + Flash + Monitor
idf.py build flash monitor

4) Note ESP32 IP Address

In logs you will see something like:

STA IP: 192.168.1.50

Connect From Client
Option A) Using netcat (Quick Test)
nc <ESP32_IP> <PORT>


Type text and press Enter → you should get a reply/echo.

Example:

nc 192.168.1.50 3333

Option B) Using Your C++ Client

Open client_TCP.cpp, set:

SERVER_IP = "<ESP32_IP>"

SERVER_PORT = <PORT>

Then compile and run (example on macOS/Linux):

g++ client_TCP.cpp -o tcp_client
./tcp_client

Demo Commands

Send: hello

Expected response: hello (echo) or an ACK message (depends on your server logic)

Notes / Troubleshooting

ESP32 and client device must be on the same Wi-Fi network

If client cannot connect:

confirm <ESP32_IP> and <PORT>

ensure firewall is not blocking the port

start server first, then client

If using ESP32-S3, ensure correct target: idf.py set-target esp32s3

Repository Contents

TCP_server.cpp (ESP32 TCP server)

client_TCP.cpp (PC TCP client)
