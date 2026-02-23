#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Device Identity ---
#define SITE_NAME   "SMK"
#define PLANT_ZONE  "HUB"
#define DEVICE_NAME "NODE01"
#define SITE_KEY    "SMK_01"

// --- Version (แยกชื่อสำหรับ Hub) ---
#define HUB_VERSION    "1.12"
#define HUB_BUILD_DATE "14-Feb-26 09:30"

// --- WiFi Settings ---
#define WIFI_SSID "MY_IOT"
#define WIFI_PASS "0819065291"

// --- OTA Settings ---
#define URL_JSON_HUB "https://raw.githubusercontent.com/thanormsinm-byte/SmartFarmOTA/main/smk-hub.json"

// --- Hardware Pins (ESP32-C3) ---
#define SDA_PIN         6   
#define SCL_PIN         7  
#define PIN_BUZR        5   
#define LED_WIFI        9  
#define LED_ALARM       10  
 
#define BTN_FORCE_OTA   20  
#define BTN_ACTIVE_LOW  true

// --- [Device Structure] ---
struct Device {
    String id;
    String ip;
    String curVer;
    String curBuild;
    String status;    
    String lastVer;   
    String lastBuild; 
    unsigned long lastTick;
    int otaProgress; // [เพิ่ม] สำหรับเก็บค่า % จากบอร์ดลูก
};

#endif
