#include <Arduino.h>
#include "btn_handler.h"
#include "config.h"

// อ้างอิงฟังก์ชันจากไฟล์หลัก
extern void playTone(int duration);

void btnInit() {
    pinMode(BTN_FORCE_OTA, BTN_ACTIVE_LOW ? INPUT_PULLUP : INPUT);
}

void btnLoop() {
    int raw = digitalRead(BTN_FORCE_OTA);
    bool pressed = (BTN_ACTIVE_LOW) ? (raw == LOW) : (raw == HIGH);

    if (pressed) {
        // --- [Print Log - ห้ามแก้/ห้ามลบ/ห้ามแถม] ---
        Serial.println("[Button] Hub Force OTA Triggered!");
        // ------------------------------------------
        
        playTone(100); // ใช้เสียง Tone แทนเสียงแกร๊ก
        
        // รอจนกว่าจะปล่อยปุ่ม
        while((BTN_ACTIVE_LOW) ? (digitalRead(BTN_FORCE_OTA) == LOW) : (digitalRead(BTN_FORCE_OTA) == HIGH)) {
            delay(10);
        }
    }
}
