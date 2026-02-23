#include "web_update_handler.h"
#include <WebServer.h>

extern WebServer server;

// ฟังก์ชันจำลองการทำงานของระบบฟาร์ม
void farmSystemInit() {
    Serial.println("[SYSTEM] Farm Logic Initialized.");
}

void farmSystemLoop() {
    // ใส่ Logic การอ่านเซนเซอร์หรือควบคุมปั๊มน้ำที่นี่
}

void webUpdateInit() {
    // หน้าอัปเดตเดิม (ถ้าจำเป็นต้องเข้า ให้เข้าผ่าน /update_old)
    server.on("/update_old", []() {
        server.send(200, "text/html", "<h1>Old Update Page</h1>");
    });
    Serial.println("[WEB] Update Handler Ready.");
}
