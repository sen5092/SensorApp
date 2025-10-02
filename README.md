# 📡 SensorApp

A modular C++17 application for simulating and interfacing with hardware sensors.  
The app supports **real camera input via OpenCV** as well as **simulation mode** with generated data.  
Collected sensor data is packaged into structured JSON and sent over UDP/TCP sockets for integration with other systems.  

---

## ✨ Features
- **Pluggable data sources**
  - `SimulationDataSource` – generates test values for development.
  - `HardwareDataSource` – captures frames from a webcam using OpenCV and extracts metadata (frame width, height, channels, brightness).
- **Networking**
  - UDP and TCP socket support for sending sensor data to a server.
- **JSON-based payloads**
  - Structured with `sensor_id`, metadata, timestamp, and readings.
  - Units included when available, using [nlohmann/json](https://github.com/nlohmann/json).
- **CMake-based build**
  - Modern target-based linking and warnings enabled.
- **Cross-platform design**
  - Works on macOS and Linux; Windows support under consideration.

---

## 📦 Dependencies
- [CMake 3.16+](https://cmake.org/)
- [nlohmann/json](https://github.com/nlohmann/json) (fetched automatically if not provided)
- [OpenCV 4.x](https://opencv.org/) (only required if building with `USE_OPENCV=ON`)

---

## ⚙️ Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourname/SensorApp.git
cd SensorApp

# Configure build (default: Release)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DUSE_OPENCV=ON

# Build
cmake --build build -j
```

If OpenCV is not found automatically, you can pass `OpenCV_DIR`:

```bash
cmake -S . -B build -DUSE_OPENCV=ON -DOpenCV_DIR=/path/to/opencv/lib/cmake/opencv4
```

---

## ▶️ Running

```bash
./build/src/Sensor
```

The app will load sensor configuration from `config/` (copied to the build tree by CMake).  
Captured data is output as line-delimited JSON and can be inspected with tools like `nc` or Wireshark.  

---

## 📊 Example JSON Payload

```json
{
  "sensor_id": "camera_1",
  "metadata": {
    "location": "lab_1"
  },
  "timestamp_ms": 1727809273562,
  "readings": {
    "frame_width":   { "value": 640, "unit": "pixels" },
    "frame_height":  { "value": 480, "unit": "pixels" },
    "channels":      { "value": 3, "unit": "count" },
    "brightness":    { "value": 123.456, "unit": "intensity" }
  }
}
```

---

## 🧪 Development & Testing
- **Strict warnings** enabled (`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion`).
- **clang-tidy** integration with rules for:
  - `cppcoreguidelines-*`
  - `modernize-*`
  - `readability-*`
- Unit tests located in `/tests`.

Run tests:

```bash
cmake --build build --target run_tests
```

---

## 🚀 Roadmap
- [ ] Better error handling and retries in socket classes.
- [ ] Unit tests for networking and OpenCV modules.
- [ ] Continuous integration pipeline with code coverage and static analysis.
- [ ] Configurable output formats (e.g. binary, Protobuf).
- [ ] Expand sensor types beyond camera input.

---

## 📜 License
MIT License. See `LICENSE` file for details.
