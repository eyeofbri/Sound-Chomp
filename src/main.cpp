// Sound-Chomp Phase 4A: navigation, signed speed and chunk-streamed live varispeed.
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
constexpr size_t kMaxSamples=kSampleRate*8, kRecordChunk=512, kPlayChunk=384;
constexpr int kBands=18,kAnalysisWindow=256,kWaveBins=204;
constexpr uint32_t kLongPressMs=650;
const float kSpeeds[]={-2.0f,-1.75f,-1.5f,-1.25f,-1.0f,-0.75f,-0.50f,0.0f,0.50f,0.75f,1.0f,1.25f,1.5f,1.75f,2.0f};
constexpr int kSpeedCount=sizeof(kSpeeds)/sizeof(kSpeeds[0]);
int speedIndex=10; // +1x
float speed(){return kSpeeds[speedIndex];}
int16_t *samples=nullptr,*playChunk=nullptr;
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
int homeSelection=0,settingsSelection=0;
enum class Page{Home,Chomp,Tracks,Settings};
enum class State{Idle,Recording,Playing,Error};
Page page=Page::Home;
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
void drawHeader(const char* title){
  int w=canvas.width();
  canvas.fillRect(2,2,w-4,17,PANEL);
  canvas.setTextSize(1);canvas.setTextColor(LIME,PANEL);canvas.setCursor(7,7);canvas.print(title);
  canvas.setTextColor(WHITE,PANEL);canvas.setCursor(w-57,7);
  canvas.printf("%s%u%%",charging?"+":"",unsigned(batteryPct));
  int batteryX=w-18;
  canvas.drawRect(batteryX,7,13,7,WHITE);
  canvas.fillRect(batteryX+2,9,(batteryPct*9)/100,3,batteryPct<20?RED:GREEN);
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
void drawChomp(){
  int w=canvas.width(),h=canvas.height();
  const char* status=state==State::Recording?"REC":state==State::Playing?"PLAY":state==State::Error?"ERROR":"READY";
  canvas.setTextColor(state==State::Recording?RED:accent(),BG);
  canvas.setTextSize(2);canvas.setCursor(7,23);canvas.print(status);
  canvas.setTextColor(accent(),BG);canvas.setCursor(w-90,23);
  if(speed()==0)canvas.print("PAUSE");else canvas.printf("%+.2fx",speed());
  canvas.setTextSize(1);canvas.setTextColor(WHITE,BG);canvas.setCursor(8,45);
  canvas.printf("%s  LOOP:%s  %.1fs",speed()<0?"REV":"FWD",loopAudio?"ON":"OFF",float(captured)/kSampleRate);
  int left=8,top=61,vw=w-16,vh=max(15,h-87);
  canvas.drawRect(left-2,top-3,vw+4,vh+6,TRIM);canvas.fillRect(left,top,vw,vh,PANEL);
  if(state==State::Recording||state==State::Playing)drawSpectrum(left,top,vw,vh);
  else if(captured>0){
    int cy=top+vh/2;canvas.drawFastHLine(left,cy,vw,TRIM);
    // Timeline compresses or expands with the chosen absolute rate.
    float rate=fabsf(speed());
    int visible=rate<0.01f?vw:constrain(int(vw*min(2.0f,1.0f/rate)),1,vw);
    for(int x=0;x<visible;x++){
      int idx=constrain(int(float(x)/visible*(kWaveBins-1)),0,kWaveBins-1);
      if(speed()<0)idx=kWaveBins-1-idx;
      int a=max(1,int(waveform[idx])*(vh/2-2)/24);
      canvas.drawFastVLine(left+x,cy-a,2*a+1,accent());
    }
  }else{
    canvas.setTextColor(MUTED,PANEL);canvas.setCursor(left+12,top+vh/2-3);canvas.print("HOLD A TO RECORD");
  }
  canvas.setTextColor(LIME,BG);canvas.setCursor(7,h-17);canvas.print("A:REC  WHEEL:RATE  CLICK:PLAY");
  canvas.setTextColor(MUTED,BG);canvas.setCursor(7,h-8);canvas.print("LONG CLICK:LOOP    B:HOME");
  if(state==State::Error){canvas.setTextColor(RED,BG);canvas.setCursor(7,h-27);canvas.print(errorText);}
}
void drawHome(){
  int w=canvas.width(),h=canvas.height();
  const char* items[]={"CHOMP","TRACKS","SETTINGS"};
  canvas.setTextColor(WHITE,BG);canvas.setCursor(8,25);canvas.print("SELECT A MODE");
  for(int i=0;i<3;i++){
    int y=40+i*24;bool sel=i==homeSelection;
    canvas.fillRect(8,y,w-16,21,sel?0x32A5:PANEL);
    canvas.drawRect(8,y,w-16,21,sel?LIME:TRIM);
    canvas.setTextColor(sel?LIME:WHITE,sel?0x32A5:PANEL);
    canvas.setCursor(16,y+7);canvas.printf("%c %s",sel?'>':' ',items[i]);
  }
  canvas.setTextColor(MUTED,BG);canvas.setCursor(7,h-10);canvas.print("TURN:SELECT  A/CLICK:OPEN");
}
void drawTracks(){
  int h=canvas.height();canvas.setTextColor(LIME,BG);canvas.setTextSize(2);canvas.setCursor(8,30);canvas.print("CHOMP 01");
  canvas.setTextSize(1);canvas.setTextColor(WHITE,BG);canvas.setCursor(8,61);
  if(captured)canvas.printf("RAM CLIP: %.2f seconds",double(captured)/kSampleRate);
  else canvas.print("NO RECORDING YET");
  canvas.setTextColor(MUTED,BG);canvas.setCursor(8,79);canvas.print("Saved tracks coming later");
  canvas.setCursor(8,h-10);canvas.print("B:BACK  A/CLICK:CHOMP");
}
void drawSettings(){
  int w=canvas.width(),h=canvas.height();
  const char* labels[]={"VOLUME","BRIGHTNESS"};
  for(int i=0;i<2;i++){
    int y=30+i*33;bool sel=settingsSelection==i;
    canvas.fillRect(8,y,w-16,29,sel?0x32A5:PANEL);
    canvas.drawRect(8,y,w-16,29,sel?(settingsEditing?YELLOW:LIME):TRIM);
    canvas.setTextColor(sel?LIME:WHITE,sel?0x32A5:PANEL);
    canvas.setCursor(15,y+5);canvas.print(labels[i]);
    canvas.setCursor(w-48,y+5);canvas.printf("%3u%%",unsigned(i==0?volumePct:brightnessPct));
    canvas.fillRect(15,y+18,(w-30)*int(i==0?volumePct:brightnessPct)/100,4,sel?LIME:GREEN);
  }
  canvas.setTextColor(WHITE,BG);canvas.setCursor(8,h-27);
  canvas.print(settingsEditing?"TURN:ADJUST CLICK:SAVE":"TURN:SELECT CLICK:EDIT");
  canvas.setTextColor(MUTED,BG);canvas.setCursor(8,h-11);canvas.print("B:BACK  SAVED AUTOMATICALLY");
}
void draw(bool force=false){
  if(!canvasReady)return;
  if(!force&&millis()-lastFrame<kFrameMs)return;
  lastFrame=millis();updateBattery();
  canvas.fillSprite(BG);canvas.drawRect(0,0,canvas.width(),canvas.height(),TRIM);
  drawHeader(page==Page::Home?"SOUND-CHOMP":page==Page::Chomp?"CHOMP / PLAYER":page==Page::Tracks?"TRACKS":"SETTINGS");
  if(page==Page::Home)drawHome();else if(page==Page::Chomp)drawChomp();
  else if(page==Page::Tracks)drawTracks();else drawSettings();
  canvas.pushSprite(0,0);
}
void handleWheel(int delta){
  if(page==Page::Home)homeSelection=constrain(homeSelection+delta,0,2);
  else if(page==Page::Chomp){
    speedIndex=constrain(speedIndex+delta,0,kSpeedCount-1);
    Serial.printf("Rate: %+.2fx (live)\n",speed());
  }else if(page==Page::Settings){
    if(settingsEditing){
      if(settingsSelection==0){volumePct=uint8_t(constrain(int(volumePct)+delta*5,0,100));setVolume();}
      else{brightnessPct=uint8_t(constrain(int(brightnessPct)+delta*5,10,100));M5.Display.setBrightness(uint8_t(int(brightnessPct)*255/100));}
    }else settingsSelection=constrain(settingsSelection+delta,0,1);
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
void saveSettings(){prefs.putUChar("volume",volumePct);prefs.putUChar("bright",brightnessPct);}
void backHome(){if(page==Page::Settings)saveSettings();settingsEditing=false;page=Page::Home;clearEvents();draw(true);}
void openSelection(){
  page=homeSelection==0?Page::Chomp:homeSelection==1?Page::Tracks:Page::Settings;
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
  M5.Mic.end();updateWaveform();if(state!=State::Error)state=State::Idle;
  setStateLed();clearEvents();draw(true);
  Serial.printf("Captured %.2fs\n",double(captured)/kSampleRate);
}
// Render a small buffer from the ORIGINAL recording at the current signed rate.
// Playback is chunk-streamed instead of pre-rendering a whole effect: wheel changes
// are applied at the next chunk boundary (~24ms of PCM + speaker queue latency).
// At zero, pause without losing the source position. On direction switch the
// source cursor continues from its current location in the opposite direction.
void playClip(){
  if(!captured||!samples||!playChunk)return;
  M5.Mic.end();
  if(!M5.Speaker.begin()){state=State::Error;errorText="SPEAKER BEGIN FAILED";return;}
  setVolume();state=State::Playing;setStateLed();clearEvents();draw(true);
  double cursor=speed()<0?double(captured-1):0.0;
  bool interrupted=false;
  while(!interrupted&&state==State::Playing){
    service();
    if(M5.BtnB.wasPressed()){interrupted=true;backHome();break;}
    if(scrollShort){scrollShort=false;interrupted=true;break;}
    if(scrollLong){scrollLong=false;loopAudio=!loopAudio;Serial.printf("Loop: %s\n",loopAudio?"ON":"OFF");}
    float rate=speed();
    if(rate==0){draw();delay(5);continue;}
    size_t count=0;
    for(;count<kPlayChunk;count++){
      if(cursor<0||cursor>=double(captured)){
        if(!loopAudio)break;
        cursor=cursor<0?double(captured-1):0.0;
      }
      size_t left=size_t(cursor);
      size_t right=min(left+1,captured-1);
      float frac=float(cursor-double(left));
      float v=samples[left]*(1.0f-frac)+samples[right]*frac;
      playChunk[count]=int16_t(v);
      cursor+=double(rate);
    }
    if(count==0)break;
    // Wait for prior chunk to finish before reusing memory. Inputs remain serviced.
    while(M5.Speaker.isPlaying()){
      service();
      if(M5.BtnB.wasPressed()){interrupted=true;backHome();break;}
      if(scrollShort){scrollShort=false;interrupted=true;break;}
      if(scrollLong){scrollLong=false;loopAudio=!loopAudio;}
      draw();delay(1);
    }
    if(interrupted)break;
    if(!M5.Speaker.playRaw(playChunk,count,kSampleRate,false,1,0)){
      state=State::Error;errorText="SPEAKER PLAY FAILED";break;
    }
    if(count>=kAnalysisWindow)analyze(playChunk,count,count/2);
    draw();
  }
  M5.Speaker.stop();M5.Speaker.end();
  if(state!=State::Error)state=State::Idle;
  setStateLed();clearEvents();draw(true);
}
} // namespace

void setup(){
  Serial.begin(115200);M5.begin(M5.config());M5.Display.setRotation(1);
  M5.Power.setExtOutput(true);Wire.begin(kGroveSda,kGroveScl);Wire.setClock(100000);
  prefs.begin("soundchomp",false);
  volumePct=uint8_t(constrain(int(prefs.getUChar("volume",65)),0,100));
  brightnessPct=uint8_t(constrain(int(prefs.getUChar("bright",80)),10,100));
  M5.Display.setBrightness(uint8_t(int(brightnessPct)*255/100));
  samples=static_cast<int16_t*>(heap_caps_malloc(kMaxSamples*sizeof(int16_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  playChunk=static_cast<int16_t*>(heap_caps_malloc(kPlayChunk*sizeof(int16_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  if(!samples||!playChunk){state=State::Error;errorText="PSRAM ALLOCATION FAILED";Serial.println(errorText);}
  canvas.setColorDepth(16);
  canvasReady=canvas.createSprite(M5.Display.width(),M5.Display.height())!=nullptr;
  if(!canvasReady){M5.Display.fillScreen(TFT_BLACK);M5.Display.drawString("CANVAS FAILED",4,4);}
  scrollConnected=probeScroll();setStateLed();updateBattery();
  Serial.println("Sound-Chomp Phase 4A - menu and live speed");draw(true);
}
void loop(){
  service();
  if(M5.BtnB.wasPressed()&&page!=Page::Home){backHome();}
  if(page==Page::Home){
    if(M5.BtnA.wasPressed()||scrollShort){openSelection();}
    scrollLong=false;
  }else if(page==Page::Chomp){
    if(state==State::Idle&&M5.BtnA.wasPressed())recordClip();
    else if(state==State::Idle&&scrollShort){scrollShort=false;playClip();}
    if(scrollLong){loopAudio=!loopAudio;scrollLong=false;}
  }else if(page==Page::Tracks){
    if(M5.BtnA.wasPressed()||scrollShort){scrollShort=false;page=Page::Chomp;draw(true);}
    scrollLong=false;
  }else if(page==Page::Settings){
    if(M5.BtnA.wasPressed()||scrollShort){scrollShort=false;settingsEditing=!settingsEditing;if(!settingsEditing)saveSettings();}
    scrollLong=false;
  }
  draw();delay(5);
}
