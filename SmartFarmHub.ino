#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <vector>
#include "esp_mac.h"
#include "config.h"
#include "WebPage.h"
#include "btn_handler.h"
#include "ota_manager.h"
#include "web_update_handler.h"

WebServer server(80);
std::vector<Device> deviceList;
String hubFullHostName = "";

// 🔥 OTA State
bool hubUpdating = false;
bool hubOTAPending = false;

// --- [Print Log - ห้ามแก้/ห้ามลบ/ห้ามแถม] ---
void logHub(String msg) { Serial.printf("[HUB-SYSTEM] %s\n", msg.c_str()); }
// ------------------------------------------

void playTone(int duration) {
    tone(PIN_BUZR, 2000);
    delay(duration);
    noTone(PIN_BUZR);
}

// ฟังก์ชันสำหรับลบอุปกรณ์ (Hub_DelDevice)
void Hub_DelDevice(String targetId) {
    for (auto it = deviceList.begin(); it != deviceList.end(); ++it) {
        if (it->id.equalsIgnoreCase(targetId)) {
            // --- [Print Log - ห้ามแก้/ห้ามลบ/ห้ามแถม] ---
            Serial.printf("[Log] Deleting Device: %s\n", targetId.c_str());
            // ------------------------------------------
            deviceList.erase(it);
            break;
        }
    }
}

void registerHubSelf() {
    String myID = hubFullHostName + " (HUB)";
    bool found = false;
    for (auto &d : deviceList) {
        if (d.id == myID) {
            d.lastTick = millis();
            found = true;
            break;
        }
    }

    if (!found) {
        Device hub;
        hub.id = myID;
        hub.ip = WiFi.localIP().toString();
        hub.curVer = HUB_VERSION;
        hub.curBuild = HUB_BUILD_DATE;
        hub.status = "Active";
        hub.otaProgress = 0;
        hub.lastTick = millis();
        deviceList.push_back(hub);
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_WIFI, OUTPUT);
    pinMode(PIN_BUZR, OUTPUT);
    btnInit();

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char macSuffix[5];
    sprintf(macSuffix, "%02X%02X", mac[4], mac[5]);

    hubFullHostName = String(SITE_NAME) + "-" +
                      String(PLANT_ZONE) + "-" +
                      String(DEVICE_NAME) + "-" +
                      String(macSuffix);
    hubFullHostName.toUpperCase();

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    digitalWrite(LED_WIFI, HIGH);
    
    MDNS.begin(hubFullHostName.c_str());
    ArduinoOTA.setHostname(hubFullHostName.c_str());
    ArduinoOTA.begin();

    registerHubSelf();

    // --- Endpoint สำหรับแสดงหน้าเว็บ ---
    server.on("/", []() {
        server.send(200, "text/html", getDashboardHTML(deviceList));
    });

    // --- Endpoint เช็คสถานะการอัปเดตของ Hub (เพื่อให้ปุ่มไม่โดน Block) ---
    server.on("/hub_state", []() {
        StaticJsonDocument<128> doc;
        doc["hubUpdating"] = hubUpdating; 
        String json;
        serializeJson(doc, json);
        server.send(200, "application/json", json);
    });

    // --- Endpoint สำหรับส่งค่า Progress Bar ให้หน้า Dashboard ---
    server.on("/hub_check_status", []() {
        String id = server.arg("id");
        int progress = 0;
        for (auto &d : deviceList) {
            if (d.id == id) {
                progress = d.otaProgress;
                break;
            }
        }
        StaticJsonDocument<128> doc;
        doc["progress"] = progress;
        String json;
        serializeJson(doc, json);
        server.send(200, "application/json", json);
    });

    // --- Endpoint รับค่า Heartbeat จากบอร์ดลูก ---
    server.on("/heartbeat", []() {
        String id  = server.arg("id");
        String ip  = server.arg("ip");
        String ver = server.arg("ver");
        String bld = server.arg("build");
        bool found = false;
        for (auto &d : deviceList) {
            if (d.id.equalsIgnoreCase(id)) {
                d.ip = ip;
                d.curVer = ver;
                d.curBuild = bld;
                d.lastTick = millis();
                d.status = "Active";
                found = true;
                break;
            }
        }
        if (!found) {
            Device dev;
            dev.id = id; dev.ip = ip; dev.curVer = ver; dev.curBuild = bld;
            dev.status = "Active"; dev.lastTick = millis(); dev.otaProgress = 0;
            deviceList.push_back(dev);
        }
        server.send(200, "text/plain", "OK");
    });

    // --- Endpoint รับ Progress การ OTA จากบอร์ดลูก ---
    server.on("/update_progress", []() {
        String id = server.arg("id");
        int p = server.arg("p").toInt();
        for (auto &d : deviceList) {
            if (d.id.equalsIgnoreCase(id)) {
                d.otaProgress = p;
                d.status = (p >= 100) ? "Success" : "Updating";
                break;
            }
        }
        server.send(200, "text/plain", "OK");
    });

    // --- Endpoint สั่งเริ่มการ OTA ---
    server.on("/execute_ota", []() {
        if (!server.hasArg("ip")) return;
        String targetIP = server.arg("ip");
        for (auto &d : deviceList) {
            if (d.ip == targetIP) {
                d.otaProgress = 0;
                d.status = "Updating";
                break;
            }
        }
        server.send(200, "text/plain", "OK");
        if (targetIP == WiFi.localIP().toString()) {
            hubOTAPending = true;
        } else {
            HTTPClient http;
            http.begin("http://" + targetIP + "/execute_ota");
            http.GET();
            http.end();
        }
    });

    // --- Endpoint ลบอุปกรณ์ (Hub_DelDevice) ---
    server.on("/delete_device", []() {
        if (server.hasArg("id")) {
            Hub_DelDevice(server.arg("id"));
            server.send(200, "text/plain", "Deleted");
        } else {
            server.send(400, "text/plain", "Bad Request");
        }
    });

    webUpdateInit();
    server.begin();
}

void loop() {
    server.handleClient();
    ArduinoOTA.handle();
    btnLoop();

    if (hubOTAPending) {
        hubOTAPending = false;
        hubUpdating = true;
        delay(200);
        processOTA(true);
    }

    static unsigned long lastHubRefresh = 0;
    if (!hubUpdating && millis() - lastHubRefresh > 5000) {
        registerHubSelf();
        // ตรวจสอบอุปกรณ์ที่หายไปเกิน 30 วินาทีให้ขึ้น Offline
        for (auto &d : deviceList) {
            if (millis() - d.lastTick > 30000 && d.status != "Offline") {
                d.status = "Offline";
            }
        }
        lastHubRefresh = millis();
    }
}
