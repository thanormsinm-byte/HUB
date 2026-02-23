#include "ota_manager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "config.h"

extern String hubFullHostName;
extern std::vector<Device> deviceList;

// --- [Print Log - ห้ามแก้/ห้ามลบ/ห้ามแถม] ---
void logOTA(String msg) { Serial.printf("[OTA-System] %s\n", msg.c_str()); }
// ------------------------------------------


// 🔥 Progress Callback (ไม่ใช้ HTTP อีกต่อไป)
void hub_progress_callback(int cur, int total) {

    static int lastPercent = -1;

    int percent = (cur * 100) / total;

    // แสดงทีละ 5%
    if (percent != lastPercent && (percent % 5 == 0 || percent == 100)) {

        lastPercent = percent;

        String myID = hubFullHostName + " (HUB)";

        for (auto &d : deviceList) {
            if (d.id == myID) {
                d.otaProgress = percent;
                d.status = (percent >= 100) ? "Success" : "Updating";
                d.lastTick = millis();
                break;
            }
        }

        Serial.printf("[HUB-OTA] Progress: %d%%\n", percent);
    }
}


void processOTA(bool force) {

    if (WiFi.status() != WL_CONNECTED) return;

    logOTA("--- Start Hub Self-Update Process ---");

    WiFiClientSecure client;
    client.setInsecure();

    String binUrl = "";

    // =========================
    // STEP 1: อ่าน JSON
    // =========================
    logOTA("Step 1: Reading JSON from: " + String(URL_JSON_HUB));

    {
        HTTPClient http;

        if (http.begin(client, URL_JSON_HUB)) {

            int httpCode = http.GET();

            if (httpCode == HTTP_CODE_OK) {

                String payload = http.getString();
                logOTA("Step 2: JSON Payload Received.");

                StaticJsonDocument<512> doc;
                DeserializationError error = deserializeJson(doc, payload);

                if (!error) {
                    binUrl = doc["url"].as<String>();
                    logOTA("Step 3: Found Target Bin URL: " + binUrl);
                } else {
                    logOTA("Step 3 Error: Cannot parse JSON -> " + String(error.c_str()));
                }

            } else {
                logOTA("Step 2 Error: HTTP Code " + String(httpCode));
            }

            http.end();
        }
    }

    // =========================
    // STEP 2: ถ้ามี bin URL
    // =========================
    if (binUrl != "" && binUrl != "null") {

        logOTA("Step 4: Verified. Starting Flash Firmware...");

        httpUpdate.onProgress(hub_progress_callback);
        httpUpdate.rebootOnUpdate(true);

        t_httpUpdate_return ret = httpUpdate.update(client, binUrl);

        if (ret == HTTP_UPDATE_FAILED) {

            Serial.printf("[OTA] Flash Failed! Error (%d): %s\n",
                          httpUpdate.getLastError(),
                          httpUpdate.getLastErrorString().c_str());

            // รีเซ็ตสถานะถ้า fail
            String myID = hubFullHostName + " (HUB)";
            for (auto &d : deviceList) {
                if (d.id == myID) {
                    d.status = "Failed";
                    break;
                }
            }
        }

    } else {
        logOTA("Step 4: Aborted. No valid Binary URL to proceed.");
    }
}

void fetchServerVersion() {
    logOTA("Checking firmware...");
}
