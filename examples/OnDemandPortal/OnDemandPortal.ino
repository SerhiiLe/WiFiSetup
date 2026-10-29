#include <Arduino.h>
#include <WiFiSetup.h>

WiFiSetup wm;

const char AP_NAME[] = "ESP-OnDemand-Setup";
const char AP_PASS[] = "12345678";

void togglePortal() {
    if (wm.getConfigPortalActive()) {
        Serial.println("Stopping Config Portal...");
        wm.stopConfigPortal();
        if (wm.getWiFiIsSaved()) {
            Serial.printf("Reconnecting to saved SSID: %s\n", wm.getWiFiSSID().c_str());
            WiFi.begin(wm.getWiFiSSID().c_str(), wm.getWiFiPass().c_str());
        }
    } else {
        Serial.println("Starting Config Portal on demand...");
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.disconnect();
        }
        wm.startConfigPortal(AP_NAME, AP_PASS);
        Serial.printf("Config Portal active at http://192.168.4.1 (AP: %s)\n", AP_NAME);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- WiFiSetup On-Demand Example ---");

    wm.setWiFiAutoReconnect(true);
    wm.setEnableConfigPortal(false); // Do not start portal automatically

    if (wm.autoConnect(15)) {
        Serial.printf("Connected to %s! IP: %s\n", wm.getWiFiSSID().c_str(), WiFi.localIP().toString().c_str());
    } else {
        Serial.println("Connection failed or no credentials saved.");
        togglePortal();
    }

    Serial.println("\nSerial Commands:");
    Serial.println(" 'p' - Toggle Config Portal on / off");
    Serial.println(" 'r' - Reset Wi-Fi credentials");
    Serial.println(" 's' - Print connection status");
}

void loop() {
    wm.process();

    if (Serial.available()) {
        char cmd = Serial.read();
        if (cmd == 'p') {
            togglePortal();
        } else if (cmd == 'r') {
            Serial.println("Erasing saved Wi-Fi settings...");
            wm.resetSettings();
        } else if (cmd == 's') {
            Serial.println("\n--- Status ---");
            Serial.printf("Connected: %s\n", (WiFi.status() == WL_CONNECTED) ? "YES" : "NO");
            Serial.printf("Portal active: %s\n", wm.getConfigPortalActive() ? "YES" : "NO");
            Serial.printf("Saved SSID: %s\n", wm.getWiFiSSID().c_str());
            if (WiFi.status() == WL_CONNECTED) {
                Serial.printf("IP: %s (RSSI: %d dBm)\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
            }
            Serial.println("--------------");
        }
    }

    delay(10);
}

