#pragma once

#include <Arduino.h>
#include <functional>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  using WebServerClass = ESP8266WebServer;
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <esp_wifi.h>
  using WebServerClass = WebServer;
#else
  #error "WiFiSetup supports only ESP8266 and ESP32 platforms."
#endif

#include <DNSServer.h>

class WiFiSetup {
public:
    using SaveCallback = std::function<void()>;

    WiFiSetup();
    ~WiFiSetup();

    // Auto-connect to saved credentials if available.
    // timeoutSec: how long to wait for connection (default 15s).
    // Returns true if connected, false otherwise.
    // If credentials are not saved or connection fails, portal is NOT started
    // automatically unless setEnableConfigPortal(true) was explicitly called.
    bool autoConnect(uint16_t timeoutSec = 15);

    // Check if Wi-Fi credentials are saved in non-volatile SDK storage
    bool getWiFiIsSaved();

    // Get stored SSID
    String getWiFiSSID();

    // Get stored Password
    String getWiFiPass();

    // Clear stored Wi-Fi credentials
    void resetSettings();

    // Enable/disable SDK Wi-Fi auto-reconnect
    void setWiFiAutoReconnect(bool autoReconnect);

    // WiFiManager compatibility stubs
    void setEnableConfigPortal(bool enable);
    void setConfigPortalBlocking(bool blocking);

    // Start Config Portal (SoftAP + Captive DNS + Web Server)
    bool startConfigPortal(const char* apSsid, const char* apPassword = nullptr);

    // Stop Config Portal (shuts down AP, DNS, Web Server, switches to STA)
    void stopConfigPortal();

    // Check if Config Portal is currently active
    bool getConfigPortalActive() const;

    // Periodic processing loop (handles DNS, WebServer, and connects on save)
    void process();

    // Callback invoked when new credentials have been submitted and saved
    void onSave(SaveCallback cb);

private:
    void _setupWebServer();
    void _handleRoot();
    void _handleScan();
    void _handleSave();
    void _handleNotFound();

    WebServerClass* _server = nullptr;
    DNSServer* _dnsServer = nullptr;

    bool _portalActive = false;
    bool _autoReconnect = true;
    bool _enableConfigPortal = false;
    bool _pendingConnect = false;
    uint32_t _pendingConnectTime = 0;

    String _targetSSID;
    String _targetPass;

    SaveCallback _onSaveCallback = nullptr;
};

