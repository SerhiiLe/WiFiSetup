#include "WiFiSetup.h"
#include "portal_html.h"

WiFiSetup::WiFiSetup() {
}

WiFiSetup::~WiFiSetup() {
    stopConfigPortal();
}

void WiFiSetup::setWiFiAutoReconnect(bool autoReconnect) {
    _autoReconnect = autoReconnect;
    WiFi.setAutoReconnect(autoReconnect);
}

void WiFiSetup::setEnableConfigPortal(bool enable) {
    _enableConfigPortal = enable;
}

void WiFiSetup::setConfigPortalBlocking(bool blocking) {
    (void)blocking; // Non-blocking by default
}

void WiFiSetup::onSave(SaveCallback cb) {
    _onSaveCallback = cb;
}

bool WiFiSetup::getWiFiIsSaved() {
    return getWiFiSSID().length() > 0;
}

#if defined(ESP8266)

String WiFiSetup::getWiFiSSID() {
    return WiFi.SSID();
}

String WiFiSetup::getWiFiPass() {
    return WiFi.psk();
}

void WiFiSetup::resetSettings() {
    WiFi.disconnect(true);
}

#elif defined(ESP32)

String WiFiSetup::getWiFiSSID() {
    wifi_config_t conf;
    if (esp_wifi_get_config(WIFI_IF_STA, &conf) == ESP_OK) {
        return String(reinterpret_cast<char*>(conf.sta.ssid));
    }
    return WiFi.SSID();
}

String WiFiSetup::getWiFiPass() {
    wifi_config_t conf;
    if (esp_wifi_get_config(WIFI_IF_STA, &conf) == ESP_OK) {
        return String(reinterpret_cast<char*>(conf.sta.password));
    }
    return WiFi.psk();
}

void WiFiSetup::resetSettings() {
    WiFi.disconnect(true, true);
}

#endif

bool WiFiSetup::autoConnect(uint16_t timeoutSec) {
    if (!getWiFiIsSaved()) {
        if (_enableConfigPortal) {
            startConfigPortal("ESP-Setup");
        }
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(_autoReconnect);
    WiFi.begin();

    uint32_t startMs = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startMs) < (timeoutSec * 1000UL)) {
        delay(100);
        yield();
    }

    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    if (_enableConfigPortal) {
        startConfigPortal("ESP-Setup");
    }
    return false;
}

bool WiFiSetup::startConfigPortal(const char* apSsid, const char* apPassword) {
    if (_portalActive) {
        return true;
    }

    WiFi.mode(WIFI_AP_STA);

    IPAddress apIP(192, 168, 4, 1);
    IPAddress netMsk(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, apIP, netMsk);

    bool apOk = false;
    if (apPassword && strlen(apPassword) >= 8) {
        apOk = WiFi.softAP(apSsid, apPassword);
    } else {
        apOk = WiFi.softAP(apSsid);
    }

    if (!apOk) {
        return false;
    }

    if (!_dnsServer) {
        _dnsServer = new DNSServer();
    }
    _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer->start(53, "*", apIP);

    if (!_server) {
        _server = new WebServerClass(80);
    }
    _setupWebServer();
    _server->begin();

    _portalActive = true;
    _pendingConnect = false;
    return true;
}

void WiFiSetup::stopConfigPortal() {
    if (!_portalActive) return;

    if (_dnsServer) {
        _dnsServer->stop();
        delete _dnsServer;
        _dnsServer = nullptr;
    }

    if (_server) {
        _server->stop();
        delete _server;
        _server = nullptr;
    }

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);

    _portalActive = false;
}

bool WiFiSetup::getConfigPortalActive() const {
    return _portalActive;
}

void WiFiSetup::process() {
    if (_portalActive) {
        if (_dnsServer) {
            _dnsServer->processNextRequest();
        }
        if (_server) {
            _server->handleClient();
        }
    }

    if (_pendingConnect && (millis() - _pendingConnectTime > 500)) {
        _pendingConnect = false;

        if (_onSaveCallback) {
            _onSaveCallback();
        }

        stopConfigPortal();

        WiFi.persistent(true);
        if (_targetPass.length() > 0) {
            WiFi.begin(_targetSSID.c_str(), _targetPass.c_str());
        } else {
            WiFi.begin(_targetSSID.c_str());
        }
    }
}

void WiFiSetup::_setupWebServer() {
    if (!_server) return;

    _server->on("/", HTTP_GET, [this]() { _handleRoot(); });
    _server->on("/scan", HTTP_GET, [this]() { _handleScan(); });
    _server->on("/save", HTTP_POST, [this]() { _handleSave(); });

    // Captive portal probes for Android, iOS, Windows
    _server->on("/generate_204", HTTP_GET, [this]() { _handleRoot(); });
    _server->on("/gen_204", HTTP_GET, [this]() { _handleRoot(); });
    _server->on("/hotspot-detect.html", HTTP_GET, [this]() { _handleRoot(); });
    _server->on("/ncsi.txt", HTTP_GET, [this]() { _server->send(200, "text/plain", "Microsoft NCSI"); });
    _server->on("/connecttest.txt", HTTP_GET, [this]() { _server->send(200, "text/plain", "Microsoft Connect Test"); });
    _server->on("/redirect", HTTP_GET, [this]() { _handleRoot(); });

    _server->onNotFound([this]() { _handleNotFound(); });
}

void WiFiSetup::_handleRoot() {
    _server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    _server->send_P(200, "text/html", PORTAL_HTML);
}

void WiFiSetup::_handleScan() {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; ++i) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0) continue;

        // Skip duplicate SSIDs
        bool duplicate = false;
        for (int j = 0; j < i; ++j) {
            if (WiFi.SSID(j) == ssid) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;

        if (json.length() > 1) json += ",";

        ssid.replace("\"", "\\\"");
        json += "{\"ssid\":\"";
        json += ssid;
        json += "\",\"rssi\":";
        json += WiFi.RSSI(i);
        json += ",\"enc\":";
        #if defined(ESP8266)
        json += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "0" : "1";
        #else
        json += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "0" : "1";
        #endif
        json += "}";
    }
    json += "]";
    WiFi.scanDelete();

    _server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    _server->send(200, "application/json", json);
}

void WiFiSetup::_handleSave() {
    if (_server->hasArg("ssid")) {
        _targetSSID = _server->arg("ssid");
        _targetPass = _server->hasArg("password") ? _server->arg("password") : "";

        _server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        _server->send(200, "application/json", "{\"status\":\"ok\"}");

        _pendingConnect = true;
        _pendingConnectTime = millis();
    } else {
        _server->send(400, "text/plain", "Missing SSID");
    }
}

void WiFiSetup::_handleNotFound() {
    String host = _server->hostHeader();
    IPAddress apIP = WiFi.softAPIP();

    if (host.length() > 0 && host != apIP.toString()) {
        _server->sendHeader("Location", String("http://") + apIP.toString() + "/", true);
        _server->send(302, "text/plain", "");
    } else {
        _handleRoot();
    }
}
