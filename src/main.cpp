// Sound-Chomp Phase 4A.3: expanded player interface and splash atop queued audio.
// SPDX-License-Identifier: MIT
#include <Arduino.h>
#include <M5Unified.h>
#include <Wire.h>
#include <Preferences.h>
#include <esp_heap_caps.h>
#include <math.h>

namespace {
constexpr uint8_t kScrollAddress=0x40, kEncoderReg=0x10, kButtonReg=0x20, kRgbReg=0x30;
constexpr int kGroveSda=9,kGroveScl=10;
constexpr uint32_t kSampleRate=16000, kPollMs=30, kFrameMs=75;
constexpr size_t kMaxSamples=kSampleRate*8, kRecordChunk=512, kPlayChunk=512, kPlayBuffers=3;
constexpr int kBands=18,kAnalysisWindow=256,kWaveBins=204;
constexpr uint32_t kLongPressMs=650, kRecordHoldMs=300;
// Each wheel tick is 0.10x. Crossing between forward and reverse skips zero.
constexpr int kStepsPerDirection=30;
constexpr int kSpeedCount=2*kStepsPerDirection;
int speedIndex=kStepsPerDirection+9; // +1.0x
bool invertedReverse=false,autoPlay=false,resetSpeedAfterRecord=true;
float speed(){
  if(speedIndex>=kStepsPerDirection)return 0.1f*float(speedIndex-kStepsPerDirection+1);
  const int magnitudeStep=invertedReverse?(speedIndex+1):(kStepsPerDirection-speedIndex);
  return -0.1f*float(magnitudeStep);
}
void resetSpeed(){ speedIndex=kStepsPerDirection+9; }
int16_t *samples=nullptr,*playChunks[kPlayBuffers]={};
size_t captured=0;
uint8_t waveform[kWaveBins]={};
float spectrum[kBands]={};
M5Canvas canvas(&M5.Display);
Preferences prefs;
bool canvasReady=false,scrollConnected=false,scrollDown=false,scrollShort=false,scrollLong=false,scrollLongHandled=false;
int32_t wheelLast=0;
uint32_t lastPoll=0,lastProbe=0,lastFrame=0,pressedAt=0,lastBatteryPoll=0;
uint8_t volumePct=65,brightnessPct=80,batteryPct=0;
bool charging=false,loopAudio=false,settingsEditing=false;
bool aWasDown=false,aHoldStarted=false;uint32_t aPressedAt=0;
int homeSelection=0,settingsSelection=0;
float playbackSeconds=0.0f;
enum class Page{Home,Chomp,Tracks,Settings,HowTo,About};
enum class State{Idle,Recording,Playing,Error};
Page page=Page::Chomp;
State state=State::Idle;
const char* errorText="";
constexpr uint16_t BG=0x0841,PANEL=0x18C3,TRIM=0x6B4D,GREEN=0x07E0,LIME=0xBFE0,PURPLE=0xB1BC,YELLOW=0xFFE0,RED=0xF800,MUTED=0xA534,WHITE=0xE71C;
uint16_t accent(){return speed()<0 ? PURPLE : GREEN;}

bool readReg(uint8_t reg,uint8_t* out,size_t n){
  Wire.beginTransmission(kScrollAddress);Wire.write(reg);
  if(Wire.endTransmission(false)!=0)return false;
  if(Wire.requestFrom(kScrollAddress,static_cast<uint8_t>(n))!=n)return false;
  for(size_t i=0;i<n;i++)out[i]=Wire.read();return true;
}
bool probeScroll(){Wire.beginTransmission(kScrollAddress);return Wire.endTransmission()==0;}
void led(uint8_t r,uint8_t g,uint8_t b){
  if(!scrollConnected)return;
  Wire.beginTransmission(kScrollAddress);Wire.write(kRgbReg);Wire.write(uint8_t(0));
  Wire.write(r);Wire.write(g);Wire.write(b);Wire.endTransmission();
}
void setStateLed(){if(state==State::Recording)led(24,0,0);else if(state==State::Playing)led(0,20,0);else led(0,0,18);}
void updateWaveform(){
  memset(waveform,0,sizeof waveform);
  if(!captured||!samples)return;
  for(int x=0;x<kWaveBins;x++){
    size_t first=captured*size_t(x)/kWaveBins;
    size_t last=max(first+1,captured*size_t(x+1)/kWaveBins);
    int peak=0;size_t step=max(size_t(1),(last-first)/32);
    for(size_t i=first;i<last&&i<captured;i+=step)peak=max(peak,abs(int(samples[i])));
    waveform[x]=uint8_t(constrain(int(peak*24.0f/32768.0f),1,24));
  }
}
void analyze(const int16_t* buffer,size_t count,size_t center){
  if(!buffer||count<kAnalysisWindow)return;
  size_t first=center>kAnalysisWindow/2?center-kAnalysisWindow/2:0;
  if(first+kAnalysisWindow>count)first=count-kAnalysisWindow;
  for(int b=0;b<kBands;b++){
    float freq=100.0f*powf(65.0f,float(b)/(kBands-1));
    float coeff=2.0f*cosf(6.2831853f*freq/float(kSampleRate));
    float p=0,p2=0;
    for(int i=0;i<kAnalysisWindow;i++){
      float win=0.5f-0.5f*cosf(6.2831853f*i/(kAnalysisWindow-1));
      float v=float(buffer[first+i])/32768.0f*win+coeff*p-p2;
      p2=p;p=v;
    }
    float energy=sqrtf(fmaxf(0.0f,p2*p2+p*p-coeff*p*p2));
    float target=constrain(log1pf(energy)/3.1f,0.0f,1.0f);
    spectrum[b]=spectrum[b]*0.46f+target*0.54f;
  }
}
void setVolume(){M5.Speaker.setVolume(uint8_t((uint16_t(volumePct)*255)/100));}
void updateBattery(){
  if(millis()-lastBatteryPoll<5000&&lastBatteryPoll!=0)return;
  lastBatteryPoll=millis();
  batteryPct=uint8_t(constrain(int(M5.Power.getBatteryLevel()),0,100));
  charging=M5.Power.isCharging();
}
void drawTeeth(int x,int y){
  // Original 12x11 two-jaw pixel glyph; deliberately distinct from reference artwork.
  const uint16_t toothColor=LIME;
  canvas.drawFastHLine(x,y,12,toothColor);
  canvas.drawFastHLine(x,y+10,12,toothColor);
  for(int i=0;i<4;i++){
    int tx=x+1+i*3;
    canvas.fillRect(tx,y+1,2,2,toothColor);
    canvas.fillRect(tx,y+8,2,2,toothColor);
    canvas.drawPixel(tx,y+3,toothColor);
    canvas.drawPixel(tx+1,y+7,toothColor);
  }
}
void drawHeader(const char* title){
  int w=canvas.width();
  canvas.fillRect(2,2,w-4,17,PANEL);
  canvas.setTextSize(1);canvas.setTextColor(LIME,PANEL);canvas.setCursor(24,7);canvas.print("SOUND-CHOMP");
  drawTeeth(7,5);
  int batteryX=w-22;
  canvas.drawRect(batteryX,7,15,8,WHITE);
  canvas.fillRect(batteryX-3,9,3,4,WHITE); // battery terminal on left
  canvas.fillRect(batteryX+2,9,(batteryPct*11)/100,4,batteryPct<20?RED:GREEN);
  canvas.setTextColor(WHITE,PANEL);canvas.setCursor(batteryX-31,7);
  canvas.printf("%u%%",unsigned(batteryPct));
  if(charging)canvas.drawPixel(batteryX+5,6,YELLOW);
}
void drawSpectrum(int left,int top,int width,int height){
  const int stride=max(3,width/kBands);
  for(int i=0;i<kBands;i++){
    int h=constrain(int(spectrum[i]*(height-2)),0,height-2);
    int x=left+i*stride;
    if(x+stride>left+width)break;
    // Winamp-style amplitude thresholds, mostly green.
    for(int yy=0;yy<h;yy+=4){
      float frac=float(yy)/float(height);
      uint16_t color=frac>0.89f?RED:(frac>0.72f?YELLOW:GREEN);
      canvas.fillRect(x,top+height-2-yy,stride-2,min(3,h-yy),color);
    }
  }
}
void drawDirectionIcon(int x,int y,bool reverse){
  // Two pixel triangles in cassette-player style.
  for(int part=0;part<2;part++){
    int ox=x+part*9;
    for(int i=0;i<7;i++){
      int v=reverse?6-i:i;
      canvas.drawFastVLine(ox+i,y+abs(3-v),7-2*abs(3-v),accent());
    }
  }
}
void drawLoopIcon(int x,int y){
  uint16_t c=loopAudio?LIME:MUTED;
  canvas.drawFastHLine(x+3,y,11,c);canvas.drawFastVLine(x+14,y,8,c);
  canvas.drawFastHLine(x+3,y+8,11,c);canvas.drawFastVLine(x+2,y+1,6,c);
  canvas.fillTriangle(x+10,y-2,x+15,y,x+10,y+3,c);
  canvas.fillRect(x+19,y+3,3,3,loopAudio?GREEN:0x2131);
}
void drawWaveform(int left,int top,int width,int height){
  const int cy=top+height/2;
  canvas.drawFastHLine(left,cy,width,TRIM);
  if(captured==0){
    canvas.setTextColor(MUTED,PANEL);canvas.setCursor(left+9,cy-3);
    canvas.print("TAP PLAY / HOLD RECORD");return;
  }
  // At 1x the clip occupies the full viewport. Faster shrinks, slower
  // stretches and is clipped. Scroll-window follows the playback cursor.
  const float rate=fmaxf(0.1f,fabsf(speed()));
  const float totalPixels=float(width)/rate;
  const float playbackProgress=(state==State::Playing && captured)
    ?constrain(playbackSeconds*rate/(float(captured)/kSampleRate),0.0f,1.0f):0.0f;
  const float startPixel=totalPixels>width?constrain(playbackProgress*totalPixels-width*0.45f,0.0f,totalPixels-width):0.0f;
  const int renderWidth=totalPixels<width?max(1,int(totalPixels)):width;
  for(int x=0;x<renderWidth;x++){
    const float fraction=(startPixel+float(x))/totalPixels;
    int idx=constrain(int(fraction*(kWaveBins-1)),0,kWaveBins-1);
    if(speed()<0)idx=kWaveBins-1-idx;
    const int amplitude=max(1,int(waveform[idx])*(height/2-2)/24);
    canvas.drawFastVLine(left+x,cy-amplitude,2*amplitude+1,accent());
  }
  if(state==State::Playing){
    int marker=int(playbackProgress*totalPixels-startPixel);
    if(marker>=0&&marker<width)canvas.drawFastVLine(left+marker,top,height,YELLOW);
  }
}
void drawChomp(){
  const int w=canvas.width(),h=canvas.height();
  // LEFT status column leaves free space for future functionality.
  if(state==State::Recording){
    canvas.fillRect(5,22,w-10,27,RED);
    // 2 Hz flash: black circle on alternating half-cycles.
    const bool flash=(millis()/250)%2==0;
    canvas.fillCircle(17,35,6,TFT_BLACK);
    if(flash)canvas.fillCircle(17,35,4,RED);
    canvas.setTextSize(2);canvas.setTextColor(TFT_BLACK,RED);
    canvas.setCursor(29,28);canvas.print("RECORDING");
  }else{
    const char* status=state==State::Playing?"PLAY":state==State::Error?"ERROR":"READY";
    canvas.setTextSize(2);canvas.setTextColor(accent(),BG);
    canvas.setCursor(8,23);canvas.print(status);
    canvas.setTextSize(1);canvas.setTextColor(accent(),BG);
    canvas.setCursor(8,46);canvas.printf("%+.2fx",double(speed()));
  }
  const int left=8,top=65,vw=w-16,vh=max(12,h-90);
  canvas.drawRect(left-2,top-3,vw+4,vh+6,TRIM);
  canvas.fillRect(left,top,vw,vh,PANEL);
  if(state==State::Recording)drawSpectrum(left,top,vw,vh);
  else if(state==State::Playing)drawSpectrum(left,top,vw,vh);
  else drawWaveform(left,top,vw,vh);
  // Fixed bottom transport row; aligns time with battery right edge.
  const int rowY=h-16;
  drawDirectionIcon(9,rowY+2,speed()<0);
  drawLoopIcon(45,rowY+2);
  canvas.setTextSize(1);canvas.setTextColor(WHITE,BG);
  const float shownTime=state==State::Playing?playbackSeconds:
      captured?float(captured)/kSampleRate/fmaxf(0.1f,fabsf(speed())):0.0f;
  char timeBuf[17];snprintf(timeBuf,sizeof timeBuf,"%.1fs",double(shownTime));
  canvas.setCursor(w-7-int(strlen(timeBuf))*6,rowY+1);canvas.print(timeBuf);
  if(state==State::Error){canvas.setTextColor(RED,BG);canvas.setCursor(85,48);canvas.print(errorText);}
}
void drawHome(){
  int w=canvas.width(),h=canvas.height();
  const char* items[]={"CHOMP","TRACKS","SETTINGS","HOW TO","ABOUT"};
  canvas.setTextColor(WHITE,BG);canvas.setCursor(8,25);canvas.print("SELECT A MODE");
  for(int i=0;i<5;i++){
    int y=35+i*17;bool sel=i==homeSelection;
    canvas.fillRect(8,y,w-16,16,sel?0x32A5:PANEL);
    canvas.drawRect(8,y,w-16,16,sel?LIME:TRIM);
    canvas.setTextColor(sel?LIME:WHITE,sel?0x32A5:PANEL);
    canvas.setCursor(16,y+4);canvas.printf("%c %s",sel?'>':' ',items[i]);
  }
  canvas.setTextColor(MUTED,BG);canvas.setCursor(7,h-10);canvas.print("TURN:SELECT  A/CLICK:OPEN");
}
void drawTracks(){
  int h=canvas.height();canvas.setTextColor(LIME,BG);canvas.setTextSize(2);canvas.setCursor(8,30);canvas.print("CHOMP 01");
  canvas.setTextSize(1);canvas.setTextColor(WHITE,BG);canvas.setCursor(8,61);
  if(captured)canvas.printf("RAM CLIP: %.2f seconds",double(captured)/kSampleRate);
  else canvas.print("NO RECORDING YET");
  canvas.setTextColor(MUTED,BG);canvas.setCursor(8,79);canvas.print("Saved tracks coming later");
  canvas.setCursor(8,h-10);canvas.print("B:MENU  A/CLICK:CHOMP");
}
void drawSettings(){
  int w=canvas.width(),h=canvas.height();
  const char* labels[]={"VOLUME","BRIGHT","REVERSE MAP","AUTO PLAY","RESET SPEED"};
  for(int i=0;i<5;i++){
    int y=22+i*19;bool sel=settingsSelection==i;
    uint16_t surface=sel?0x32A5:PANEL;
    canvas.fillRect(7,y,w-14,18,surface);
    canvas.drawRect(7,y,w-14,18,sel?(settingsEditing?YELLOW:LIME):TRIM);
    canvas.setTextColor(sel?LIME:WHITE,surface);canvas.setTextSize(1);
    canvas.setCursor(13,y+5);canvas.print(labels[i]);
    canvas.setCursor(w-75,y+5);
    if(i==0)canvas.printf("%u%%",unsigned(volumePct));
    else if(i==1)canvas.printf("%u%%",unsigned(brightnessPct));
    else if(i==2)canvas.print(invertedReverse?"INVERTED":"NORMAL");
    else if(i==3)canvas.print(autoPlay?"ON":"OFF");
    else canvas.print(resetSpeedAfterRecord?"ON":"OFF");
  }
  canvas.setTextColor(WHITE,BG);canvas.setCursor(7,h-14);
  canvas.print(settingsEditing?"TURN:CHANGE CLICK:DONE":"TURN:SELECT CLICK:EDIT");
}

void drawInfoPage(bool about){
  canvas.setTextSize(1);canvas.setTextColor(LIME,BG);
  if(about){
    canvas.setCursor(8,29);canvas.print("SOUND-CHOMP v0.3.1-dev");
    canvas.setTextColor(WHITE,BG);canvas.setCursor(8,47);canvas.print("BUILT BY u/eyeofbri");
    canvas.setCursor(8,65);canvas.print("github.com/eyeofbri/");
    canvas.setCursor(8,75);canvas.print("Sound-Chomp");
  }else{
    canvas.setCursor(8,26);canvas.print("CHOMP: A TAP = PLAY / STOP");
    canvas.setCursor(8,40);canvas.print("A HOLD = RECORD; RELEASE=STOP");
    canvas.setCursor(8,54);canvas.print("TURN WHEEL = SPEED / REVERSE");
    canvas.setCursor(8,68);canvas.print("WHEEL CLICK = PLAY / STOP");
    canvas.setCursor(8,82);canvas.print("WHEEL HOLD = LOOP ON / OFF");
    canvas.setCursor(8,96);canvas.print("B = MENU");
  }
  canvas.setTextColor(MUTED,BG);canvas.setCursor(8,canvas.height()-13);canvas.print("B: MENU");
}
void draw(bool force=false){
  if(!canvasReady)return;
  if(!force&&millis()-lastFrame<kFrameMs)return;
  lastFrame=millis();updateBattery();
  canvas.fillSprite(BG);canvas.drawRect(0,0,canvas.width(),canvas.height(),TRIM);
  drawHeader(page==Page::Home?"SOUND-CHOMP":page==Page::Chomp?"CHOMP / PLAYER":page==Page::Tracks?"TRACKS":"SETTINGS");
  if(page==Page::Home)drawHome();else if(page==Page::Chomp)drawChomp();
  else if(page==Page::Tracks)drawTracks();else if(page==Page::Settings)drawSettings();else drawInfoPage(page==Page::About);
  canvas.pushSprite(0,0);
}
void handleWheel(int delta){
  if(page==Page::Home)homeSelection=constrain(homeSelection+delta,0,4);
  else if(page==Page::Chomp){
    speedIndex=constrain(speedIndex+delta,0,kSpeedCount-1);
    Serial.printf("Rate: %+.2fx (live)\n",speed());
  }else if(page==Page::Settings){
    if(settingsEditing){
      if(settingsSelection==0){volumePct=uint8_t(constrain(int(volumePct)+delta*5,0,100));setVolume();}
      else if(settingsSelection==1){brightnessPct=uint8_t(constrain(int(brightnessPct)+delta*5,10,100));M5.Display.setBrightness(uint8_t(int(brightnessPct)*255/100));}
      else if(settingsSelection==2 && delta!=0)invertedReverse=delta>0;
      else if(settingsSelection==3 && delta!=0)autoPlay=delta>0;
      else if(settingsSelection==4 && delta!=0)resetSpeedAfterRecord=delta>0;
    }else settingsSelection=constrain(settingsSelection+delta,0,4);
  }
}
void updateScroll(){
  uint32_t now=millis();if(now-lastPoll<kPollMs)return;lastPoll=now;
  if(!scrollConnected){
    if(now-lastProbe<1000)return;lastProbe=now;scrollConnected=probeScroll();
    if(scrollConnected){uint8_t v[4];if(readReg(kEncoderReg,v,4)){
      uint32_t n=uint32_t(v[0])|(uint32_t(v[1])<<8)|(uint32_t(v[2])<<16)|(uint32_t(v[3])<<24);
      wheelLast=int32_t(n);
    }setStateLed();}return;
  }
  uint8_t v[4],button=1;
  if(!readReg(kEncoderReg,v,4)||!readReg(kButtonReg,&button,1)){
    scrollConnected=false;scrollDown=false;Serial.println("Scroll disconnected");return;
  }
  uint32_t bits=uint32_t(v[0])|(uint32_t(v[1])<<8)|(uint32_t(v[2])<<16)|(uint32_t(v[3])<<24);
  int32_t pos=int32_t(bits);
  if(pos!=wheelLast){int32_t d=pos-wheelLast;if(d>-32&&d<32)handleWheel(int(d));}
  wheelLast=pos;
  bool down=button==0;
  if(down&&!scrollDown){pressedAt=now;scrollLongHandled=false;}
  if(down&&!scrollLongHandled&&now-pressedAt>=kLongPressMs){scrollLong=true;scrollLongHandled=true;}
  if(!down&&scrollDown&&!scrollLongHandled)scrollShort=true;
  scrollDown=down;
}
void service(){M5.update();updateScroll();}
void clearEvents(){scrollShort=false;scrollLong=false;}
void saveSettings(){prefs.putUChar("volume",volumePct);prefs.putUChar("bright",brightnessPct);prefs.putBool("revInvert",invertedReverse);prefs.putBool("autoPlay",autoPlay);prefs.putBool("resetSpd",resetSpeedAfterRecord);}
void backHome(){aWasDown=false;aHoldStarted=false;if(page==Page::Settings)saveSettings();settingsEditing=false;page=Page::Home;clearEvents();draw(true);}
void openSelection(){
  aWasDown=M5.BtnA.isPressed();aHoldStarted=true;
  page=homeSelection==0?Page::Chomp:homeSelection==1?Page::Tracks:homeSelection==2?Page::Settings:homeSelection==3?Page::HowTo:Page::About;
  clearEvents();draw(true);
}
void recordClip(){
  if(!samples)return;
  captured=0;M5.Speaker.end();
  if(!M5.Mic.begin()){state=State::Error;errorText="MIC BEGIN FAILED";return;}
  state=State::Recording;setStateLed();clearEvents();draw(true);
  while(captured<kMaxSamples){
    if(!M5.BtnA.isPressed()||M5.BtnB.wasPressed())break;
    size_t count=min(kRecordChunk,kMaxSamples-captured);
    if(!M5.Mic.record(samples+captured,count,kSampleRate)){
      state=State::Error;errorText="MIC RECORD FAILED";break;
    }
    while(M5.Mic.isRecording()){service();draw();delay(1);}
    captured+=count;analyze(samples,captured,captured-count/2);service();draw();
  }
  M5.Mic.end();updateWaveform();if(resetSpeedAfterRecord)resetSpeed();if(state!=State::Error)state=State::Idle;
  setStateLed();clearEvents();draw(true);
  Serial.printf("Captured %.2fs\n",double(captured)/kSampleRate);
}
// Generate output from the original PCM at the selected signed speed.
// IMPORTANT: keep a queue of audio ahead of the speaker, and do not modify
// a buffer until at least two other buffers have been submitted. M5Unified
// documents three rotating buffers as a safe strategy for generated audio.
void playClip(){
  if(!captured||!samples)return;
  for(auto* b:playChunks)if(!b)return;
  M5.Mic.end();
  if(!M5.Speaker.begin()){state=State::Error;errorText="SPEAKER BEGIN FAILED";return;}
  setVolume();state=State::Playing;setStateLed();clearEvents();draw(true);
  double cursor=speed()<0?double(captured-1):0.0;
  playbackSeconds=0.0f;
  bool interrupted=false,sourceExhausted=false;
  bool playADown=M5.BtnA.isPressed();uint32_t playAPressed=millis();
  unsigned queued=0;
  size_t chunksMade=0;
  constexpr uint8_t kChannel=0;
  while(!interrupted&&state==State::Playing){
    service();
    if(M5.BtnB.wasPressed()){interrupted=true;backHome();break;}
    // Tapping A during playback stops playback, just like wheel click.
    const bool buttonDown=M5.BtnA.isPressed();
    if(buttonDown&&!playADown)playAPressed=millis();
    if(!buttonDown&&playADown&&millis()-playAPressed<kRecordHoldMs){interrupted=true;break;}
    playADown=buttonDown;
    if(scrollShort){scrollShort=false;interrupted=true;break;}
    if(scrollLong){scrollLong=false;loopAudio=!loopAudio;}

    // Only submit when the speaker has a free queue slot. The two-slot
    // queue keeps the following segment ready before the first finishes.
    // A third backing buffer prevents overwriting speaker-owned PCM.
    const size_t occupied=M5.Speaker.isPlaying(kChannel);
    const float rate=speed();
    if(!sourceExhausted && rate!=0 && occupied<2){
      int16_t* output=playChunks[queued%kPlayBuffers];
      size_t count=0;
      for(;count<kPlayChunk;count++){
        if(cursor<0 || cursor>=double(captured)){
          if(!loopAudio){sourceExhausted=true;break;}
          cursor=cursor<0?double(captured-1):0.0;
        }
        const size_t left=size_t(cursor);
        const size_t right=min(left+1,captured-1);
        const float frac=float(cursor-double(left));
        const float v=samples[left]*(1.0f-frac)+samples[right]*frac;
        output[count]=int16_t(v);
        cursor+=double(rate);
      }
      if(count){
        if(!M5.Speaker.playRaw(output,count,kSampleRate,false,1,kChannel,false)){
          state=State::Error;errorText="SPEAKER PLAY FAILED";break;
        }
        ++queued;playbackSeconds+=float(count)/float(kSampleRate);
        // The Goertzel analysis can be expensive; do it on only every
        // fourth chunk, rather than on every 32ms playback chunk.
        if((chunksMade++%4)==0 && count>=kAnalysisWindow){
          analyze(output,count,count/2);
        }
      }
    }
    if(sourceExhausted && !M5.Speaker.isPlaying(kChannel))break;
    // Keep button/scroll responsiveness without doing heavy UI work
    // between every generated PCM segment. draw() is frame-limited.
    draw();
    delay(1);
  }
  M5.Speaker.stop();M5.Speaker.end();
  if(state!=State::Error)state=State::Idle;
  setStateLed();clearEvents();draw(true);
}
void splash(){
  if(!canvasReady)return;
  const int w=canvas.width(),h=canvas.height();
  const uint32_t beginning=millis();
  while(millis()-beginning<1050){
    service();
    if(M5.BtnA.isPressed()||M5.BtnB.isPressed()||scrollShort||scrollDown)break;
    const uint32_t elapsed=millis()-beginning;
    const float travel=fminf(1.0f,float(elapsed)/360.0f);
    const int upperY=int(-35+travel*(h/2+25));
    const int lowerY=int(h+35-travel*(h/2+25));
    canvas.fillSprite(BG);
    const int startX=w/2-44;
    canvas.drawFastHLine(startX,upperY-10,88,LIME);
    canvas.drawFastHLine(startX,lowerY+10,88,LIME);
    for(int i=0;i<7;i++){
      canvas.fillTriangle(startX+4+i*12,upperY-9,startX+13+i*12,upperY-9,startX+9+i*12,upperY+4,LIME);
      canvas.fillTriangle(startX+4+i*12,lowerY+9,startX+13+i*12,lowerY+9,startX+9+i*12,lowerY-4,LIME);
    }
    canvas.setTextSize(2);canvas.setTextColor(WHITE,BG);
    canvas.setCursor(max(0,w/2-66),h/2-8);canvas.print("SOUND-CHOMP");
    canvas.pushSprite(0,0);
    delay(17);
  }
  clearEvents();
}
} // namespace

void setup(){
  Serial.begin(115200);M5.begin(M5.config());M5.Display.setRotation(1);
  M5.Power.setExtOutput(true);Wire.begin(kGroveSda,kGroveScl);Wire.setClock(100000);
  prefs.begin("soundchomp",false);
  volumePct=uint8_t(constrain(int(prefs.getUChar("volume",65)),0,100));
  brightnessPct=uint8_t(constrain(int(prefs.getUChar("bright",80)),10,100));
  invertedReverse=prefs.getBool("revInvert",false);autoPlay=prefs.getBool("autoPlay",false);
  resetSpeedAfterRecord=prefs.getBool("resetSpd",true);
  M5.Display.setBrightness(uint8_t(int(brightnessPct)*255/100));
  samples=static_cast<int16_t*>(heap_caps_malloc(kMaxSamples*sizeof(int16_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  for(auto& b:playChunks)b=static_cast<int16_t*>(heap_caps_malloc(kPlayChunk*sizeof(int16_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  bool audioBuffersReady=samples!=nullptr;for(auto* b:playChunks)audioBuffersReady=audioBuffersReady&&(b!=nullptr);
  if(!audioBuffersReady){state=State::Error;errorText="PSRAM ALLOCATION FAILED";Serial.println(errorText);}
  canvas.setColorDepth(16);
  canvasReady=canvas.createSprite(M5.Display.width(),M5.Display.height())!=nullptr;
  if(!canvasReady){M5.Display.fillScreen(TFT_BLACK);M5.Display.drawString("CANVAS FAILED",4,4);}
  scrollConnected=probeScroll();setStateLed();updateBattery();
  Serial.println("Sound-Chomp Phase 4A.3 - layout refresh");splash();draw(true);
}
void loop(){
  service();
  if(M5.BtnB.wasPressed()&&page!=Page::Home){backHome();}
  if(page==Page::Home){
    if(M5.BtnA.wasPressed()||scrollShort){openSelection();}
    scrollLong=false;
  }else if(page==Page::Chomp){
    // Distinguish tap from hold. A release after recording is never a tap.
    const bool down=M5.BtnA.isPressed();
    if(down&&!aWasDown){aPressedAt=millis();aHoldStarted=false;}
    if(down&&!aHoldStarted&&millis()-aPressedAt>=kRecordHoldMs){
      aHoldStarted=true;
      recordClip(); // blocking capture loop; release stops it
      if(autoPlay&&state==State::Idle&&captured>0)playClip();
    }
    if(!down&&aWasDown&&!aHoldStarted&&millis()-aPressedAt<kRecordHoldMs&&state==State::Idle){
      playClip();
    }
    aWasDown=down;
    if(state==State::Idle&&scrollShort){scrollShort=false;playClip();}
    if(scrollLong){loopAudio=!loopAudio;scrollLong=false;}
  }else if(page==Page::Tracks){
    if(M5.BtnA.wasPressed()||scrollShort){scrollShort=false;page=Page::Chomp;draw(true);}
    scrollLong=false;
  }else if(page==Page::Settings){
    if(M5.BtnA.wasPressed()||scrollShort){scrollShort=false;settingsEditing=!settingsEditing;if(!settingsEditing)saveSettings();}
    scrollLong=false;
  }else if(page==Page::HowTo||page==Page::About){
    scrollShort=false;scrollLong=false;
  }
  draw();delay(5);
}
