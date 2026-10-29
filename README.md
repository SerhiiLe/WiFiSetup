# WiFiSetup

Lightweight, non-blocking captive portal Wi-Fi configuration library for **ESP8266** and **ESP32** (including ESP32-C3).

Designed as a clean, minimal, memory-efficient replacement for `WiFiManager`.

---

## Features

- **Lightweight & Compact**: Source HTML/CSS/JS is stored in Flash as a gzip-compressed byte array (~1.9 KB, saving ~70% flash space, 0 bytes of RAM allocated when idle).
- **Asynchronous Network Scan**: The portal UI opens instantly; visible networks are fetched asynchronously via `/scan` with duplicate filtering, signal strength (%) and lock indicators.
- **Native SDK Storage**: Uses built-in Wi-Fi storage in SDK (Flash / NVS) with zero third-party storage overhead.
- **Non-blocking by default**: Never halts your main loop or displays (ideal for LED matrices, clocks, FastLED animations).
- **On-the-fly AP switching**: Start or stop the captive portal at any time during runtime without rebooting the microcontroller.
- **Full Captive Portal**: Built-in DNS server and probe redirection for Android, iOS, macOS, and Windows.
- **Web Source & Packaging Tool**: The original, formatted HTML/CSS/JS is kept in `web/portal.html`. A python script `tools/pack_web.py` compresses and packages it into `src/portal_html.h`.

---

## Project Structure

```text
WiFiSetup/
├── library.json         # PlatformIO library manifest
├── README.md
├── web/
│   └── portal.html      # Original, editable HTML/CSS/JS source
├── tools/
│   └── pack_web.py      # Python script to minify, gzip, and generate portal_html.h
└── src/
    ├── WiFiSetup.h      # Main header file
    ├── WiFiSetup.cpp    # Core logic
    └── portal_html.h    # Auto-generated gzipped PROGMEM byte array
```

---

## Editing & Packaging the Web Page

1. Edit [`web/portal.html`](file:///home/serj/Документи/PlatformIO/Projects/WiFiSetup-test/lib/WiFiSetup/web/portal.html) in any editor or browser.
2. Run the packaging tool:
   ```bash
   python3 lib/WiFiSetup/tools/pack_web.py
   ```
3. The script will automatically minify, gzip-compress, and regenerate [`src/portal_html.h`](file:///home/serj/Документи/PlatformIO/Projects/WiFiSetup-test/lib/WiFiSetup/src/portal_html.h).

---

## API Reference

```cpp
#include <WiFiSetup.h>

WiFiSetup wm;

// Try connecting to saved credentials (timeout in seconds, default: 15)
bool connected = wm.autoConnect(15);

// Check if credentials exist in SDK storage
bool saved = wm.getWiFiIsSaved();

// Retrieve saved SSID and Password
String ssid = wm.getWiFiSSID();
String pass = wm.getWiFiPass();

// Clear saved credentials
wm.resetSettings();

// Enable SDK auto-reconnect
wm.setWiFiAutoReconnect(true);

// Start Captive Portal manually
wm.startConfigPortal("MyDevice-Setup", "optional_password");

// Stop Captive Portal manually
wm.stopConfigPortal();

// Check if portal is currently running
bool active = wm.getConfigPortalActive();

// Call in loop() to handle DNS and HTTP requests
wm.process();

// Optional callback when new credentials are submitted
wm.onSave([]() {
    Serial.println("New WiFi credentials received!");
});
```

---

## Migration from `WiFiManager`

To migrate from `tzapu/WiFiManager`, simply change the include:

```diff
- #include <WiFiManager.h>
- WiFiManager wm;
+ #include <WiFiSetup.h>
+ WiFiSetup wm;
```

All standard methods (`autoConnect`, `getWiFiIsSaved`, `startConfigPortal`, `stopConfigPortal`, `getConfigPortalActive`, `getWiFiSSID`, `getWiFiPass`, `resetSettings`, `process`, `setWiFiAutoReconnect`) work identically!
