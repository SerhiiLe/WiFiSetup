#include <Arduino.h>
#include <WiFiSetup.h>

WiFiSetup wm;

const char AP_NAME[] = "ESP-WiFi-Setup";
const char AP_PASS[] = "12345678"; // Set to nullptr for open AP

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- WiFiSetup Basic Example ---");

    // Optional callback when user saves new credentials via web portal
    wm.onSave([]() {
        Serial.println("[Callback] New Wi-Fi credentials received!");
    });

    // Try connecting to saved Wi-Fi (timeout 15 seconds)
    if (wm.autoConnect(15)) {
        Serial.println("Wi-Fi connected successfully!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Could not connect to saved network. Starting Config Portal...");
        // Start non-blocking captive portal
        wm.startConfigPortal(AP_NAME, AP_PASS);
        Serial.print("Connect to AP: ");
        Serial.println(AP_NAME);
        Serial.println("Web portal address: http://192.168.4.1");
    }
}

void loop() {
    // Process captive portal DNS, HTTP requests and pending connections
    wm.process();

    static bool lastConnected = false;
    bool connected = (WiFi.status() == WL_CONNECTED);

    if (connected && !lastConnected) {
        lastConnected = true;
        Serial.println("Wi-Fi connected!");
        Serial.print("Local IP: ");
        Serial.println(WiFi.localIP());
    } else if (!connected && lastConnected) {
        lastConnected = false;
        Serial.println("Wi-Fi disconnected!");
    }

    // Your main loop code runs smoothly without being blocked
    delay(10);
}

