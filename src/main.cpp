// Sound-Chomp Phase 2: one-clip microphone recorder and playback.
// SPDX-License-Identifier: MIT
#include <Arduino.h>
#include <M5Unified.h>
#include <Wire.h>
#include <esp_heap_caps.h>

namespace {
constexpr uint8_t kScrollAddress = 0x40;
constexpr uint8_t kEncoderReg = 0x10;
constexpr uint8_t kButtonReg = 0x20;
constexpr uint8_t kRgbReg = 0x30;
constexpr int kGroveSda = 9;
constexpr int kGroveScl = 10;
constexpr uint32_t kSampleRate = 16000;
constexpr size_t kMaxSeconds = 8;
constexpr size_t kMaxSamples = kSampleRate * kMaxSeconds;
constexpr size_t kChunkSamples = 512;
constexpr uint32_t kPollMs = 35;

int16_t* samples = nullptr;
size_t captured = 0;
bool scrollConnected = false;
bool scrollDown = false;
bool scrollClick = false;
int32_t wheelLast = 0;
int selection = 0;  // 0 = Record, 1 = Play
uint32_t lastPoll = 0;
uint32_t lastProbe = 0;
uint32_t lastDraw = 0;
enum class Mode { Idle, Recording, Playing, Error };
Mode mode = Mode::Idle;
const char* errorMessage = nullptr;

bool readRegister(uint8_t reg, uint8_t* buffer, size_t count) {
  Wire.beginTransmission(kScrollAddress);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(kScrollAddress, static_cast<uint8_t>(count)) != count) return false;
  for (size_t i = 0; i < count; ++i) buffer[i] = Wire.read();
  return true;
}

bool probeScroll() {
  Wire.beginTransmission(kScrollAddress);
  return Wire.endTransmission() == 0;
}

void setLed(uint8_t r, uint8_t g, uint8_t b) {
  if (!scrollConnected) return;
  Wire.beginTransmission(kScrollAddress);
  Wire.write(kRgbReg);
  Wire.write(static_cast<uint8_t>(0));
  Wire.write(r);
  Wire.write(g);
  Wire.write(b);
  Wire.endTransmission();
}

void updateScroll() {
  const auto now = millis();
  if (now - lastPoll < kPollMs) return;
  lastPoll = now;
  if (!scrollConnected) {
    if (now - lastProbe < 1000) return;
    lastProbe = now;
    scrollConnected = probeScroll();
    if (scrollConnected) {
      uint8_t raw[4];
      if (readRegister(kEncoderReg, raw, 4)) {
        uint32_t bits = uint32_t(raw[0]) | (uint32_t(raw[1]) << 8) |
                        (uint32_t(raw[2]) << 16) | (uint32_t(raw[3]) << 24);
        wheelLast = static_cast<int32_t>(bits);
      }
      setLed(0, 0, 18);
    }
    return;
  }
  uint8_t raw[4];
  uint8_t button;
  if (!readRegister(kEncoderReg, raw, 4) || !readRegister(kButtonReg, &button, 1)) {
    scrollConnected = false;
    scrollDown = false;
    Serial.println("Unit Scroll disconnected; retrying");
    return;
  }
  uint32_t bits = uint32_t(raw[0]) | (uint32_t(raw[1]) << 8) |
                  (uint32_t(raw[2]) << 16) | (uint32_t(raw[3]) << 24);
  const int32_t position = static_cast<int32_t>(bits);
  if (mode == Mode::Idle && position != wheelLast) {
    selection = (position > wheelLast) ? 1 : 0;
    Serial.printf("Selected: %s\n", selection == 0 ? "Record" : "Play");
  }
  wheelLast = position;
  const bool pressed = (button == 0);
  if (pressed && !scrollDown) scrollClick = true;
  scrollDown = pressed;
}

void drawScreen(bool force = false) {
  if (!force && millis() - lastDraw < 140) return;
  lastDraw = millis();
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(6, 4);
  M5.Display.println("SOUND-CHOMP");
  M5.Display.setTextSize(1);
  M5.Display.setCursor(6, 32);
  if (mode == Mode::Recording) M5.Display.setTextColor(TFT_RED, TFT_BLACK);
  else if (mode == Mode::Playing) M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  else if (mode == Mode::Error) M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5.Display.printf("%s\n", mode == Mode::Recording ? "RECORDING" :
                              mode == Mode::Playing ? "PLAYING" :
                              mode == Mode::Error ? "ERROR" : "READY");
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.printf("Clip: %.1fs / %us\n", double(captured) / kSampleRate,
                    static_cast<unsigned>(kMaxSeconds));
  if (mode == Mode::Idle) {
    M5.Display.printf("Wheel: %s\n", selection == 0 ? "[REC]  PLAY" : " REC  [PLAY]");
    M5.Display.println("Press wheel to select");
    M5.Display.println("A=REC  B=PLAY");
  } else if (mode == Mode::Recording) {
    M5.Display.println("Wheel/A = stop");
  } else if (mode == Mode::Playing) {
    M5.Display.println("Wheel/B = stop");
  } else {
    M5.Display.println(errorMessage ? errorMessage : "Unknown failure");
  }
  M5.Display.printf("Scroll: %s\n", scrollConnected ? "OK" : "missing");
}

void serviceInputs() {
  M5.update();
  updateScroll();
}

void recordClip() {
  if (!samples) return;
  Serial.println("Recording started");
  captured = 0; // Replace previous clip.
  M5.Speaker.end();
  if (!M5.Mic.begin()) {
    errorMessage = "Mic.begin failed";
    mode = Mode::Error;
    Serial.println(errorMessage);
    return;
  }
  mode = Mode::Recording;
  setLed(24, 0, 0);
  scrollClick = false;
  drawScreen(true);
  while (captured < kMaxSamples) {
    // Mic.record queues one chunk; wait for completion before reusing storage.
    const size_t count = min(kChunkSamples, kMaxSamples - captured);
    if (!M5.Mic.record(samples + captured, count, kSampleRate)) {
      errorMessage = "Mic.record failed";
      mode = Mode::Error;
      Serial.println(errorMessage);
      break;
    }
    while (M5.Mic.isRecording()) {
      serviceInputs();
      drawScreen();
      delay(1);
    }
    captured += count; // The queued chunk finished, so these samples are valid.
    serviceInputs();
    drawScreen();
    if (scrollClick || M5.BtnA.wasPressed()) break;
  }
  M5.Mic.end();
  scrollClick = false;
  if (mode != Mode::Error) mode = Mode::Idle;
  setLed(0, 0, 18);
  Serial.printf("Recording finished: %u samples, %.2fs\n",
                static_cast<unsigned>(captured), double(captured) / kSampleRate);
  drawScreen(true);
}

void playClip() {
  if (captured == 0 || !samples) {
    Serial.println("No clip available. Record first.");
    return;
  }
  M5.Mic.end();
  if (!M5.Speaker.begin()) {
    errorMessage = "Speaker.begin failed";
    mode = Mode::Error;
    Serial.println(errorMessage);
    return;
  }
  M5.Speaker.setVolume(160);
  if (!M5.Speaker.playRaw(samples, captured, kSampleRate, false, 1, 0)) {
    errorMessage = "playRaw failed";
    mode = Mode::Error;
    M5.Speaker.end();
    Serial.println(errorMessage);
    return;
  }
  mode = Mode::Playing;
  scrollClick = false;
  setLed(0, 22, 0);
  Serial.println("Playback started");
  drawScreen(true);
  while (M5.Speaker.isPlaying()) {
    serviceInputs();
    if (scrollClick || M5.BtnB.wasPressed()) {
      M5.Speaker.stop();
      break;
    }
    drawScreen();
    delay(5);
  }
  M5.Speaker.end();
  mode = Mode::Idle;
  scrollClick = false;
  setLed(0, 0, 18);
  Serial.println("Playback finished");
  drawScreen(true);
}
} // namespace

void setup() {
  Serial.begin(115200);
  M5.begin(M5.config());
  M5.Display.setRotation(1);
  M5.Power.setExtOutput(true);
  Wire.begin(kGroveSda, kGroveScl);
  Wire.setClock(100000);
  samples = static_cast<int16_t*>(heap_caps_malloc(kMaxSamples * sizeof(int16_t),
                                                    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (!samples) {
    errorMessage = "PSRAM allocation failed";
    mode = Mode::Error;
    Serial.println(errorMessage);
  }
  scrollConnected = probeScroll();
  if (scrollConnected) setLed(0, 0, 18);
  Serial.println("Sound-Chomp Phase 2: record/play, 16kHz 16-bit mono, 8 seconds");
  drawScreen(true);
}

void loop() {
  serviceInputs();
  if (mode == Mode::Idle) {
    const bool clicked = scrollClick;
    scrollClick = false;
    if (M5.BtnA.wasPressed() || (clicked && selection == 0)) recordClip();
    else if (M5.BtnB.wasPressed() || (clicked && selection == 1)) playClip();
  }
  drawScreen();
  delay(5);
}
