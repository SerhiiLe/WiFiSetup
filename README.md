# WiFiSetup

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/slebedenko/library/WiFiSetup.svg)](https://registry.platformio.org/libraries/slebedenko/WiFiSetup)
[![Framework](https://img.shields.io/badge/Framework-Arduino-blue)](https://www.arduino.cc/)
[![Platform ESP8266](https://img.shields.io/badge/Platform-ESP8266-orange.svg)](https://espressif.com/)
[![Platform ESP32](https://img.shields.io/badge/Platform-ESP32-red.svg)](https://espressif.com/)

**WiFiSetup** is a lightweight, non-blocking captive portal Wi-Fi configuration library for **ESP8266** and **ESP32** (including ESP32-C3, S2, S3).

Designed as a modern, minimal, memory-efficient replacement for `WiFiManager`.

---

## ⚡ Key Highlights

* **Ultra-compact Flash Footprint**: Web assets are stored in Flash as a gzip-compressed byte array (~1.9 KB), reducing flash usage by ~70%.
* **Zero RAM Overhead**: Stored strictly in `PROGMEM`. Consumes 0 bytes of heap memory when portal is idle.
* **Non-blocking by Design**: Never freezes your main loop or displays (ideal for LED matrices, FastLED animations, clocks, and sensors).
* **On-the-fly AP Switching**: Start or stop the captive portal at any time during runtime without rebooting the microcontroller.
* **Instant Page Load & Asynchronous Scan**: The portal UI opens in milliseconds; Wi-Fi networks are scanned asynchronously via `/scan` with mesh duplicate filtering, signal strength (%) and lock indicators.
* **Native SDK Storage**: Uses built-in SDK storage (Flash / NVS) with zero third-party dependencies or custom EEPROM layouts.
* **Full Captive Portal**: Built-in DNS server with automatic probe redirection for Android, iOS, macOS, and Windows.
* **Editable Web UI & Packaging Tool**: The source HTML/CSS/JS is cleanly formatted in `web/portal.html`. A python script `tools/pack_web.py` minifies and packs it into `src/portal_html.h`.

---

## 📊 Comparison with WiFiManager

| Feature | WiFiManager (tzapu) | WiFiSetup |
| :--- | :---: | :---: |
| **Flash Overhead** | Large (thousands of lines, multi-template) | **Minimal (~1.9 KB compressed UI)** |
| **Heap / RAM Usage** | High (multiple dynamic `String` allocations) | **0 bytes RAM idle (PROGMEM gzip)** |
| **Default Mode** | Blocking | **Non-blocking (async `process()`)** |
| **Reboot on Save** | Usually requires reboot | **Connects on the fly, no reboot required** |
| **Web UI Scan** | Synchronous (browser delays on load) | **Asynchronous (`fetch('/scan')`)** |
| **On-demand AP Toggle** | Cumbersome | **Built-in `startConfigPortal` / `stopConfigPortal`** |

---

## 💻 Hardware Support

- **ESP8266** (NodeMCU, Wemos D1 Mini, ESP-01, ESP-12, etc.)
- **ESP32** (DevKit, ESP32-WROOM, ESP32-WROVER)
- **ESP32-C3 / S2 / S3**

---

## 📦 Installation

### PlatformIO

Add `WiFiSetup` to your `platformio.ini`:

```ini
lib_deps =
    https://github.com/SerhiiLe/WiFiSetup.git
```

*Or via PlatformIO Registry (once published):*
```ini
lib_deps =
    SerhiiLe/WiFiSetup
```

### Arduino IDE

1. Download the repository as a ZIP archive from GitHub.
2. In Arduino IDE, navigate to: **Sketch** -> **Include Library** -> **Add .ZIP Library...**
3. Select the downloaded ZIP file.

---

## 🚀 Quick Start

```cpp
#include <Arduino.h>
#include <WiFiSetup.h>

WiFiSetup wm;

void setup() {
    Serial.begin(115200);

    // Try connecting to saved network (15-second timeout)
    if (wm.autoConnect(15)) {
        Serial.println("Wi-Fi connected!");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Starting Config Portal...");
        // Starts AP: "ESP-Setup" with password "12345678"
        wm.startConfigPortal("ESP-Setup", "12345678");
    }
}

void loop() {
    // Keep captive portal and DNS alive
    wm.process();

    if (WiFi.status() == WL_CONNECTED) {
        // Your connected code here
    }

    delay(10);
}
```

---

## 📖 API Reference

### Methods

| Method | Description |
| :--- | :--- |
| `bool autoConnect(uint16_t timeoutSec = 15)` | Attempts connection to saved credentials. Returns `true` on success, `false` on failure/timeout. Does not start portal unless enabled. |
| `bool getWiFiIsSaved()` | Checks whether Wi-Fi credentials (SSID) exist in non-volatile SDK storage. |
| `String getWiFiSSID()` | Returns the stored Wi-Fi SSID. |
| `String getWiFiPass()` | Returns the stored Wi-Fi password. |
| `void resetSettings()` | Clears saved Wi-Fi credentials from SDK storage. |
| `void setWiFiAutoReconnect(bool enable)` | Enables or disables SDK automatic reconnection. |
| `bool startConfigPortal(const char* apSsid, const char* apPassword = nullptr)` | Starts SoftAP, Captive DNS server, and WebServer. |
| `void stopConfigPortal()` | Stops SoftAP, DNS, and WebServer, returning to STA mode. |
| `bool getConfigPortalActive() const` | Returns `true` if Config Portal is currently active. |
| `void process()` | Must be called in `loop()` to handle DNS, HTTP client requests, and connect on save. |
| `void onSave(std::function<void()> cb)` | Registers a callback invoked when new credentials are submitted via web portal. |
| `void setEnableConfigPortal(bool enable)` | Compatibility method with WiFiManager. |
| `void setConfigPortalBlocking(bool blocking)` | Compatibility method (portal is non-blocking by design). |

---

## 🎨 Web UI & Customization

The web interface is located in [`web/portal.html`](web/portal.html). It features:
* Responsive dark theme (mobile-first).
* Live Wi-Fi scan with signal strength (%) and lock indicators (`🔒`).
* Instant network selection on click.
* Password visibility toggle (`👁`).

### Repackaging the Web Interface

Whenever you modify `web/portal.html`, update the embedded C header by running the python tool:

```bash
python3 tools/pack_web.py
```

The script will automatically:
1. Strip comments and whitespace.
2. Gzip-compress the content (`compresslevel=9`).
3. Generate [`src/portal_html.h`](src/portal_html.h).

---

## 🔄 Migration from `WiFiManager`

Replacing `tzapu/WiFiManager` in existing code is completely drop-in:

```diff
- #include <WiFiManager.h>
- WiFiManager wm;
+ #include <WiFiSetup.h>
+ WiFiSetup wm;
```

All common methods (`autoConnect`, `getWiFiIsSaved`, `startConfigPortal`, `stopConfigPortal`, `getConfigPortalActive`, `getWiFiSSID`, `getWiFiPass`, `resetSettings`, `process`, `setWiFiAutoReconnect`) work identically!

---

## 📁 Repository Structure

```text
WiFiSetup/
├── library.json             # PlatformIO Registry metadata
├── library.properties       # Arduino Library Registry metadata
├── keywords.txt             # Arduino IDE syntax highlighting
├── LICENSE                  # GNU LGPL v3 License
├── README.md                # Documentation
├── web/
│   └── portal.html          # Editable source HTML/CSS/JS
├── tools/
│   └── pack_web.py          # Assets minifier and gzip packer
├── src/
│   ├── WiFiSetup.h          # Public header
│   ├── WiFiSetup.cpp        # Implementation
│   └── portal_html.h        # Auto-generated gzipped PROGMEM array
└── examples/
    ├── Basic/
    │   └── Basic.ino        # Basic auto-connect and captive portal example
    └── OnDemandPortal/
        └── OnDemandPortal.ino # On-demand runtime portal activation example
```

---

## 📄 License

This library is licensed under the **GNU Lesser General Public License v3.0 (LGPL-3.0)**.  
See the [LICENSE](LICENSE) file for complete details.

---

## 👤 Author

**Serhii Lebedenko**  
- GitHub: [@SerhiiLe](https://github.com/SerhiiLe)
- Email: slebedenko@gmail.com
