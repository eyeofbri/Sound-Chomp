// Sound-Chomp phase 1: StickS3 + Unit Scroll hardware smoke test.
// SPDX-License-Identifier: MIT
#include <Arduino.h>
#include <M5Unified.h>
#include <Wire.h>

namespace {
constexpr uint8_t kScrollAddress = 0x40;
constexpr uint8_t kEncoderReg = 0x10;
constexpr uint8_t kButtonReg = 0x20;
constexpr uint8_t kRgbReg = 0x30;
constexpr int kGroveSda = 9;
constexpr int kGroveScl = 10;
constexpr uint32_t kPollIntervalMs = 60;

bool scrollConnected = false;
int32_t encoderValue = 0;
bool scrollPressed = false;
uint32_t lastPoll = 0;
uint32_t lastProbe = 0;
uint32_t lastDraw = 0;

bool probeScroll() {
  Wire.beginTransmission(kScrollAddress);
  return Wire.endTransmission() == 0;
}

bool readRegister(uint8_t reg, uint8_t* buffer, size_t length) {
  Wire.beginTransmission(kScrollAddress);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(kScrollAddress, static_cast<uint8_t>(length)) != length) {
    while (Wire.available()) Wire.read();
    return false;
  }
  for (size_t i = 0; i < length; ++i) buffer[i] = Wire.read();
  return true;
}

void setScrollLed(uint8_t r, uint8_t g, uint8_t b) {
  if (!scrollConnected) return;
  Wire.beginTransmission(kScrollAddress);
  Wire.write(kRgbReg);
  Wire.write(static_cast<uint8_t>(0)); // LED index
  Wire.write(r);
  Wire.write(g);
  Wire.write(b);
  Wire.endTransmission();
}

void drawStatus() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(6, 6);
  M5.Display.println("SOUND-CHOMP");
  M5.Display.setTextSize(1);
  M5.Display.println("Phase 1 hardware test");
  M5.Display.println();
  M5.Display.printf("Scroll: %s\n", scrollConnected ? "FOUND" : "NOT FOUND");
  M5.Display.printf("Wheel: %ld\n", static_cast<long>(encoderValue));
  M5.Display.printf("Press: %s\n", scrollPressed ? "DOWN" : "UP");
  M5.Display.printf("Btn A: %s\n", M5.BtnA.isPressed() ? "DOWN" : "UP");
  M5.Display.printf("Btn B: %s\n", M5.BtnB.isPressed() ? "DOWN" : "UP");
  M5.Display.println("USB serial: 115200");
}
} // namespace

void setup() {
  Serial.begin(115200);
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);
  // StickS3 defaults the Grove external 5 V rail to input/off.
  M5.Power.setExtOutput(true);
  Wire.begin(kGroveSda, kGroveScl);
  Wire.setClock(100000);
  delay(100);
  scrollConnected = probeScroll();
  if (scrollConnected) setScrollLed(0, 24, 0);
  Serial.println("Sound-Chomp Phase 1 hardware test");
  Serial.printf("Unit Scroll at 0x40: %s\n", scrollConnected ? "found" : "not found");
  drawStatus();
}

void loop() {
  M5.update();
  const uint32_t now = millis();
  if (now - lastPoll >= kPollIntervalMs) {
    lastPoll = now;
    if (!scrollConnected && now - lastProbe >= 1000) {
      lastProbe = now;
      scrollConnected = probeScroll();
      if (scrollConnected) setScrollLed(0, 24, 0);
    }
    if (scrollConnected) {
      uint8_t raw[4];
      uint8_t btn;
      if (readRegister(kEncoderReg, raw, sizeof(raw)) && readRegister(kButtonReg, &btn, 1)) {
        const uint32_t bits = static_cast<uint32_t>(raw[0]) |
                              (static_cast<uint32_t>(raw[1]) << 8) |
                              (static_cast<uint32_t>(raw[2]) << 16) |
                              (static_cast<uint32_t>(raw[3]) << 24);
        const int32_t newEncoder = static_cast<int32_t>(bits);
        const bool newPressed = (btn == 0);
        if (newEncoder != encoderValue || newPressed != scrollPressed) {
          encoderValue = newEncoder;
          scrollPressed = newPressed;
          Serial.printf("Wheel=%ld Press=%d\n", static_cast<long>(encoderValue), scrollPressed);
          setScrollLed(scrollPressed ? 24 : 0, scrollPressed ? 0 : 24, 0);
        }
      } else {
        scrollConnected = false;
        Serial.println("Unit Scroll read failed; retrying discovery");
      }
    }
  }
  if (now - lastDraw >= 150) {
    lastDraw = now;
    drawStatus();
  }
  delay(5);
}
