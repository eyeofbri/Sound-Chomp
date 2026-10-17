// Sound-Chomp v0.3.0 prototype: speed, reverse, loop + double-buffered-style canvas UI.
// SPDX-License-Identifier: MIT
#include <Arduino.h>
#include <M5Unified.h>
#include <Wire.h>
#include <esp_heap_caps.h>
#include <math.h>

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
constexpr size_t kMaxProcessed = kMaxSamples * 2;  // 0.5x = twice as many output samples.
constexpr size_t kChunkSamples = 512;
constexpr uint32_t kPollMs = 35;
constexpr uint32_t kLongPressMs = 650;

int16_t* samples = nullptr;
int16_t* processed = nullptr;
size_t captured = 0;
bool scrollConnected = false;
bool scrollDown = false;
bool scrollShortClick = false;
bool scrollLongClick = false;
bool scrollLongHandled = false;
uint32_t scrollPressedAt = 0;
int32_t wheelLast = 0;
uint32_t lastPoll = 0, lastProbe = 0, lastDraw = 0;
int speedStep = 4; // range 0..12 => 0.5x..2.0x, 1.0x default
bool reverseAudio = false, loopAudio = false;
enum class Mode { Idle, Recording, Playing, Error };
Mode mode = Mode::Idle;
const char* errorMessage = nullptr;
M5Canvas canvas(&M5.Display);
bool canvasReady = false;

// Explicit values allow predictable behavior when scrolling at either extreme.
constexpr float kSpeedValues[] = {0.50f, 0.625f, 0.75f, 0.875f, 1.0f,
                                  1.125f, 1.25f, 1.375f, 1.50f,
                                  1.625f, 1.75f, 1.875f, 2.0f};
float speed() { return kSpeedValues[speedStep]; }

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
  Wire.write(r); Wire.write(g); Wire.write(b);
  Wire.endTransmission();
}
void updateScroll() {
  const uint32_t now = millis();
  if (now - lastPoll < kPollMs) return;
  lastPoll = now;
  if (!scrollConnected) {
    if (now - lastProbe < 1000) return;
    lastProbe = now;
    scrollConnected = probeScroll();
    if (scrollConnected) {
      uint8_t raw[4];
      if (readRegister(kEncoderReg, raw, 4)) {
        const uint32_t bits = uint32_t(raw[0]) | (uint32_t(raw[1]) << 8) |
                              (uint32_t(raw[2]) << 16) | (uint32_t(raw[3]) << 24);
        wheelLast = static_cast<int32_t>(bits);
      }
      setLed(0, 0, 18);
    }
    return;
  }
  uint8_t raw[4]; uint8_t button = 1;
  if (!readRegister(kEncoderReg, raw, 4) || !readRegister(kButtonReg, &button, 1)) {
    scrollConnected = false;
    scrollDown = false;
    Serial.println("Unit Scroll disconnected; retrying");
    return;
  }
  const uint32_t bits = uint32_t(raw[0]) | (uint32_t(raw[1]) << 8) |
                        (uint32_t(raw[2]) << 16) | (uint32_t(raw[3]) << 24);
  const int32_t position = static_cast<int32_t>(bits);
  if (position != wheelLast) {
    const int32_t delta = position - wheelLast;
    // Guard against reconnect wrap or implausible jumps.
    if (delta > -32 && delta < 32) {
      speedStep = constrain(speedStep + static_cast<int>(delta), 0, 12);
      Serial.printf("Speed: %.3fx%s\n", speed(), mode == Mode::Playing ? " (next play/loop)" : "");
    }
  }
  wheelLast = position;
  const bool pressed = (button == 0);
  if (pressed && !scrollDown) {
    scrollPressedAt = now;
    scrollLongHandled = false;
  }
  if (pressed && !scrollLongHandled && now - scrollPressedAt >= kLongPressMs) {
    scrollLongClick = true;
    scrollLongHandled = true;
  }
  if (!pressed && scrollDown && !scrollLongHandled) scrollShortClick = true;
  scrollDown = pressed;
}

void uiFrame() {
  // Rendering is always composed offscreen before a single transfer to the LCD.
  // Avoid calling M5.Display.fillScreen() in the running loop.
  const int w = canvas.width(), h = canvas.height();
  canvas.fillSprite(0x0841); // deep charcoal
  canvas.drawRect(0, 0, w, h, 0x8410);
  canvas.fillRect(3, 3, w - 6, 16, 0x2945);
  canvas.setTextColor(0xFFFF, 0x2945);
  canvas.setTextSize(1);
  canvas.setCursor(8, 7);
  canvas.print("SOUND-CHOMP  [03A]");
  const uint16_t green = 0x07E0, amber = 0xFFE0, muted = 0x9CD3;
  canvas.setTextColor(green, 0x0841);
  canvas.setCursor(7, 25);
  canvas.printf("%s  %.2fx", mode == Mode::Recording ? "REC" : mode == Mode::Playing ? "PLAY" :
                        mode == Mode::Error ? "ERR" : "READY", speed());
  canvas.setCursor(7, 38);
  canvas.printf("CLIP %0.1fs  %s", double(captured) / kSampleRate,
                reverseAudio ? "REV" : "FWD");
  canvas.setTextColor(amber, 0x0841);
  canvas.setCursor(7, 51);
  canvas.printf("LOOP %s   SCROLL %s", loopAudio ? "ON" : "OFF", scrollConnected ? "OK" : "--");
  // Winamp-inspired tiny green level bars: static when idle; intentionally not fake audio analysis.
  const int barY = 72;
  for (int i = 0; i < 27; ++i) {
    const int v = (i * 7 + 11) % 16;
    canvas.fillRect(7 + i * 5, barY - v, 3, v, mode == Mode::Recording ? 0xF800 : green);
  }
  canvas.drawFastHLine(6, 78, w - 12, 0x7BEF);
  canvas.setTextColor(muted, 0x0841);
  canvas.setCursor(7, 83);
  canvas.print("A REC  B PLAY  PRESS REV");
  canvas.setCursor(7, 95);
  canvas.print("HOLD: LOOP  TURN: SPEED");
  if (mode == Mode::Error) {
    canvas.setTextColor(0xFBE0, 0x0841);
    canvas.setCursor(7, h - 12);
    canvas.print(errorMessage ? errorMessage : "Error");
  }
  canvas.pushSprite(0, 0);
}
void drawScreen(bool force = false) {
  if (!canvasReady) return;
  if (!force && millis() - lastDraw < 120) return;
  lastDraw = millis();
  uiFrame();
}
void serviceInputs() { M5.update(); updateScroll(); }
void clearScrollEvents() { scrollShortClick = false; scrollLongClick = false; }

void recordClip() {
  if (!samples) return;
  captured = 0;
  M5.Speaker.end();
  if (!M5.Mic.begin()) {
    errorMessage = "Mic.begin failed"; mode = Mode::Error; Serial.println(errorMessage); return;
  }
  mode = Mode::Recording;
  setLed(24, 0, 0);
  clearScrollEvents();
  drawScreen(true);
  Serial.println("Recording started");
  while (captured < kMaxSamples) {
    const size_t count = min(kChunkSamples, kMaxSamples - captured);
    if (!M5.Mic.record(samples + captured, count, kSampleRate)) {
      errorMessage = "Mic.record failed"; mode = Mode::Error; Serial.println(errorMessage); break;
    }
    while (M5.Mic.isRecording()) {
      serviceInputs(); drawScreen(); delay(1);
    }
    captured += count;
    serviceInputs(); drawScreen();
    if (scrollShortClick || scrollLongClick || M5.BtnA.wasPressed()) break;
  }
  M5.Mic.end();
  clearScrollEvents();
  if (mode != Mode::Error) mode = Mode::Idle;
  setLed(0, 0, 18);
  Serial.printf("Recording finished: %u samples (%.2fs)\n",
                static_cast<unsigned>(captured), double(captured) / kSampleRate);
  drawScreen(true);
}

// Construct a new PCM clip without touching the original recording.
// Linear interpolation handles fractional speed ratios; reversal maps source indices.
size_t renderPlaybackBuffer() {
  if (!captured || !processed) return 0;
  const double ratio = speed();
  const size_t frames = min(kMaxProcessed, static_cast<size_t>(captured / ratio));
  for (size_t i = 0; i < frames; ++i) {
    double pos = i * ratio;
    size_t left = static_cast<size_t>(pos);
    if (left >= captured) left = captured - 1;
    const size_t right = min(left + 1, captured - 1);
    const float frac = static_cast<float>(pos - left);
    const size_t a = reverseAudio ? captured - 1 - left : left;
    const size_t b = reverseAudio ? captured - 1 - right : right;
    const float mixed = samples[a] * (1.0f - frac) + samples[b] * frac;
    processed[i] = static_cast<int16_t>(mixed);
  }
  return frames;
}

void playClip() {
  if (!captured || !samples || !processed) {
    Serial.println("No clip available. Record first."); return;
  }
  M5.Mic.end();
  if (!M5.Speaker.begin()) {
    errorMessage = "Speaker.begin failed"; mode = Mode::Error; Serial.println(errorMessage); return;
  }
  M5.Speaker.setVolume(160);
  mode = Mode::Playing;
  clearScrollEvents();
  setLed(0, 22, 0);
  bool interrupted = false;
  do {
    const size_t frames = renderPlaybackBuffer();
    if (!frames || !M5.Speaker.playRaw(processed, frames, kSampleRate, false, 1, 0)) {
      errorMessage = "playRaw failed"; mode = Mode::Error; Serial.println(errorMessage); break;
    }
    Serial.printf("Play: %.2fx %s loop=%s samples=%u\n", speed(),
                  reverseAudio ? "reverse" : "forward", loopAudio ? "on" : "off",
                  static_cast<unsigned>(frames));
    drawScreen(true);
    while (M5.Speaker.isPlaying()) {
      serviceInputs();
      if (scrollShortClick || M5.BtnB.wasPressed()) {
        M5.Speaker.stop(); interrupted = true; break;
      }
      // A long press switches looping for the next iteration.
      if (scrollLongClick) {
        loopAudio = !loopAudio;
        scrollLongClick = false;
        Serial.printf("Loop: %s\n", loopAudio ? "ON" : "OFF");
      }
      drawScreen(); delay(5);
    }
  } while (!interrupted && loopAudio && mode != Mode::Error);
  M5.Speaker.end();
  if (mode != Mode::Error) mode = Mode::Idle;
  clearScrollEvents();
  setLed(0, 0, 18);
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
  samples = static_cast<int16_t*>(heap_caps_malloc(kMaxSamples * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  processed = static_cast<int16_t*>(heap_caps_malloc(kMaxProcessed * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (!samples || !processed) {
    errorMessage = "PSRAM allocation failed"; mode = Mode::Error; Serial.println(errorMessage);
  }
  canvas.setColorDepth(16);
  canvasReady = canvas.createSprite(M5.Display.width(), M5.Display.height()) != nullptr;
  if (!canvasReady) {
    Serial.println("Canvas allocation failed (no flicker-free UI available)");
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setCursor(4, 4);
    M5.Display.println("Canvas allocation failed");
  }
  scrollConnected = probeScroll();
  if (scrollConnected) setLed(0, 0, 18);
  Serial.println("Sound-Chomp Phase 3A: effects and canvas UI");
  drawScreen(true);
}
void loop() {
  serviceInputs();
  if (mode == Mode::Idle) {
    if (scrollLongClick) {
      loopAudio = !loopAudio; scrollLongClick = false;
      Serial.printf("Loop: %s\n", loopAudio ? "ON" : "OFF");
    }
    if (scrollShortClick) {
      reverseAudio = !reverseAudio; scrollShortClick = false;
      Serial.printf("Direction: %s\n", reverseAudio ? "REVERSE" : "FORWARD");
    }
    if (M5.BtnA.wasPressed()) recordClip();
    else if (M5.BtnB.wasPressed()) playClip();
  }
  drawScreen();
  delay(5);
}
