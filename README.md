# tcp-data-visualizer

Client–server project for streaming numerical data, computing statistics and visualizing it in a graph.

## Project description

This project consists of two parts:

1) **Python TCP server**
2) **C TCP client with GTK + Cairo visualization**

The **server**:
- opens TCP socket on `localhost:65432`
- accepts one client connection
- generates random integers in range 0..100
- sends them line by line with a small delay
- emulates telemetry or sensor data stream

The **client**:
- connects to the TCP server
- receives up to 1000 integer values
- computes:
  - mean value
  - median
  - standard deviation
  - moving average
- displays:
  - raw data curve (blue)
  - moving average curve (red)
- visualization implemented using GTK and Cairo

## Technologies

- C
- Python
- WinSock2
- Sockets (TCP)
- GTK
- Cairo
- basic statistics

## How to run

### 1. Start the server

```bash
python server.py

### 2. Build and run the client (Windows example)

```bash
gcc client.c -o client.exe `pkg-config --cflags --libs gtk+-3.0` -lws2_32
./client.exe

## Result

- statistics are printed in console:
  - mean
  - median
  - standard deviation
- GTK window opens
- graph is displayed:
  - blue curve — raw data
  - red curve — moving average

## Example Use Case

This project can be used to visualize real-time data streams from:
- sensors
- telemetry systems
- training simulators
- laboratory experiments

It is suitable for educational projects and research prototypes.

## Disclaimer

This project is intended for educational and research purposes only.
The author is not responsible for any misuse of the provided code.
