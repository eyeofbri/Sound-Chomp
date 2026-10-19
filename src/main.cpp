// Sound-Chomp Phase 5E: Winamp-style controls, effects and adaptive layout.
// SPDX-License-Identifier: MIT
#include <Arduino.h>
#include <M5Unified.h>
#include <Wire.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <esp_heap_caps.h>
#include <math.h>
#include "generated/ui_assets.h"
#include "branding.h"

namespace {
constexpr uint8_t kScrollAddress=0x40, kEncoderReg=0x10, kButtonReg=0x20, kRgbReg=0x30;
constexpr int kGroveSda=9,kGroveScl=10;
constexpr uint32_t kSampleRate=16000, kPollMs=30, kFrameMs=75;
constexpr size_t kMaxSamples=kSampleRate*8, kRecordChunk=512, kPlayChunk=512, kPlayBuffers=3;
constexpr int kBands=18,kAnalysisWindow=256,kWaveBins=204;
constexpr uint32_t kLongPressMs=650, kRecordHoldMs=300;
// Each wheel tick is 0.10x; ±0.50x is the minimum audible rate.
// Crossing forward/reverse skips the distorted 0..0.49x region.
constexpr int kStepsPerDirection=26; // 0.50x through 3.00x in 0.10x steps
constexpr int kSpeedCount=2*kStepsPerDirection;
int speedIndex=kStepsPerDirection+5; // +1.0x
bool invertedReverse=false,autoPlay=false,resetSpeedAfterRecord=true;
// Echo is mixed only into speaker output; stored LittleFS clips are unchanged.
constexpr int kEchoMinMs=100,kEchoMaxMs=600,kEchoStepMs=50;
constexpr size_t kEchoBufferSamples=(kSampleRate*kEchoMaxMs)/1000;
int16_t* echoHistory=nullptr;
size_t echoHead=0;
uint8_t echoWet=35;
uint16_t echoDelayMs=250;
bool echoEnabled=false;
// All effects are non-destructive: only synthesized speaker PCM is processed.
bool stutterEnabled=false, robotEnabled=false;
uint16_t stutterSliceMs=180; // repeated slice duration (80-400 ms)
uint8_t robotIntensity=65; // square-wave tremolo wet mix (0-100%)
uint32_t robotPhase=0;
constexpr uint32_t kRobotHz=55;
constexpr size_t kStutterCapacity=kSampleRate*400/1000;
int16_t* stutterHistory=nullptr;
size_t stutterPos=0, stutterFilled=0;
uint32_t effectSampleClock=0;
constexpr const char* kFirmwareVersion="v0.7.0-dev";
enum class PlayerFocus:uint8_t{Play=0,Speed=1,EchoToggle=2,Stutter=3,Robot=4,Loop=5};
PlayerFocus playerFocus=PlayerFocus::Play;
bool playerEditing=false; // Green selection -> amber adjustment
enum class OrientationMode:uint8_t{Landscape=0,Portrait=1,Auto=2};
OrientationMode orientationMode=OrientationMode::Landscape;
uint8_t displayRotation=1;
uint8_t portraitRotation=0;
uint32_t lastOrientationPoll=0, orientationStableAt=0;
uint8_t orientationCandidate=1;
uint8_t effectsField=0; // 0=enabled,1=mix,2=delay
void advancePlayerFocus(){playerFocus=static_cast<PlayerFocus>((static_cast<int>(playerFocus)+1)%6);}

int effectsSelection=0; // Echo, Stutter, Robot
bool effectsEditing=false;
bool effectPreferenceDirty=false;
uint32_t effectPreferenceChangedAt=0;
int16_t applyEcho(int16_t dry){
  if(!echoHistory)return dry;
  const size_t delaySamples=size_t(echoDelayMs)*kSampleRate/1000;
  const size_t readIndex=(echoHead+kEchoBufferSamples-delaySamples)%kEchoBufferSamples;
  // Delay-line feedback is deliberately low to avoid runaway echoes.
  const int32_t delayed=echoHistory[readIndex];
  const int32_t wet=echoEnabled?int32_t(echoWet):0;
  const int32_t mixed=int32_t(dry)+(delayed*wet)/100;
  const int32_t feedback=int32_t(dry)+(delayed*35)/100;
  echoHistory[echoHead]=int16_t(constrain(feedback,-32768,32767));
  echoHead=(echoHead+1)%kEchoBufferSamples;
  return int16_t(constrain(mixed,-32768,32767));
}
void resetEchoHistory(){if(echoHistory)memset(echoHistory,0,kEchoBufferSamples*sizeof(int16_t));echoHead=0;}
void resetPlaybackEffects(){
  resetEchoHistory();
  stutterPos=stutterFilled=0;effectSampleClock=0;robotPhase=0;
  if(stutterHistory)memset(stutterHistory,0,kStutterCapacity*sizeof(int16_t));
}
int16_t applyLiveEffects(int16_t dry){
  // Repeating grain: capture one original slice, then replay that slice
  // rather than advancing an entire track while the effect is engaged.
  if(stutterEnabled && stutterHistory){
    const size_t slice=constrain(size_t(stutterSliceMs)*kSampleRate/1000,size_t(1),kStutterCapacity);
    if(stutterFilled<slice)stutterHistory[stutterFilled++]=dry;
    if(stutterFilled==slice){dry=stutterHistory[stutterPos%slice];stutterPos=(stutterPos+1)%slice;}
  }else {stutterFilled=0;stutterPos=0;}
  // Robot voice: 55Hz square-wave amplitude modulation with dry/wet mix.
  if(robotEnabled){
    const int gain=((robotPhase / (kSampleRate/(2*kRobotHz)))&1u)?-1:1;
    robotPhase=(robotPhase+1)%(kSampleRate/(kRobotHz)*2);
    const int32_t robot=int32_t(dry)*gain;
    dry=int16_t(constrain((int32_t(dry)*(100-robotIntensity)+robot*robotIntensity)/100,-32768,32767));
  }
  ++effectSampleClock;
  return applyEcho(dry);
}

float speed(){
  if(speedIndex>=kStepsPerDirection)return 0.5f+0.1f*float(speedIndex-kStepsPerDirection);
  const int magnitudeStep=invertedReverse?(speedIndex):(kStepsPerDirection-1-speedIndex);
  return -(0.5f+0.1f*float(magnitudeStep));
}
void resetSpeed(){ speedIndex=kStepsPerDirection+5; }
constexpr int kTrackCount=4;
struct Track {int16_t* pcm=nullptr;size_t length=0;};
Track tracks[kTrackCount];
int activeTrack=0,trackSelection=0;
bool trackDeleteConfirm=false;
bool storageReady=false;
bool storageSaveFailed=false;
// A small binary header rejects truncated, incompatible and corrupt clips.
struct ClipHeader { uint32_t magic, version, sampleRate, sampleCount, checksum; };
constexpr uint32_t kClipMagic=0x504D4853; // "SHMP" marker
constexpr uint32_t kClipVersion=1;
static_assert(sizeof(ClipHeader)==20,"Unexpected clip header layout");
void trackPath(int which, char* out, size_t n, bool temporary=false){
  snprintf(out,n,"/chomp%02d.%s",which+1,temporary?"tmp":"pcm");
}
uint32_t clipChecksum(const int16_t* data,size_t n){
  // FNV-1a over each PCM byte; inexpensive and endian-consistent on ESP32.
  uint32_t hash=2166136261u;
  const uint8_t* b=reinterpret_cast<const uint8_t*>(data);
  for(size_t i=0;i<n*sizeof(int16_t);++i){hash^=b[i];hash*=16777619u;}
  return hash;
}
bool writeAll(File& f,const uint8_t* src,size_t n){
  while(n){size_t written=f.write(src,min(n,size_t(4096)));if(!written)return false;src+=written;n-=written;}
  return true;
}
bool saveTrack(int which){
  if(!storageReady||which<0||which>=kTrackCount||!tracks[which].pcm)return false;
  char dest[24],tmp[24];trackPath(which,dest,sizeof dest);trackPath(which,tmp,sizeof tmp,true);
  LittleFS.remove(tmp);
  File f=LittleFS.open(tmp,"w");
  if(!f){Serial.println("Storage: cannot create temporary clip");return false;}
  const Track& t=tracks[which];
  ClipHeader hdr{kClipMagic,kClipVersion,kSampleRate,uint32_t(t.length),clipChecksum(t.pcm,t.length)};
  bool ok=writeAll(f,reinterpret_cast<const uint8_t*>(&hdr),sizeof hdr);
  if(ok&&t.length)ok=writeAll(f,reinterpret_cast<const uint8_t*>(t.pcm),t.length*sizeof(int16_t));
  f.flush();f.close();
  File verify=LittleFS.open(tmp,"r");
  ok=ok&&verify&&verify.size()==sizeof hdr+t.length*sizeof(int16_t);
  if(verify)verify.close();
  if(!ok){LittleFS.remove(tmp);Serial.println("Storage: failed to write clip");return false;}
  // Keep the prior recording until the replacement was written completely.
  // Filesystem rename over an existing destination varies by implementation.
  LittleFS.remove(dest);
  if(!LittleFS.rename(tmp,dest)){Serial.println("Storage: rename failed");return false;}
  Serial.printf("Saved track %d, %u samples\n",which+1,unsigned(t.length));
  return true;
}
void deleteSavedTrack(int which){
  if(!storageReady)return;char dest[24],tmp[24];
  trackPath(which,dest,sizeof dest);trackPath(which,tmp,sizeof tmp,true);
  LittleFS.remove(dest);LittleFS.remove(tmp);
}
void loadTracks(){
  if(!storageReady)return;
  for(int i=0;i<kTrackCount;i++){
    char path[24];trackPath(i,path,sizeof path);
    File f=LittleFS.open(path,"r");
    if(!f)continue;
    ClipHeader hdr{};
    const bool headerOK=f.read(reinterpret_cast<uint8_t*>(&hdr),sizeof hdr)==sizeof hdr &&
       hdr.magic==kClipMagic && hdr.version==kClipVersion &&
       hdr.sampleRate==kSampleRate && hdr.sampleCount<=kMaxSamples &&
       f.size()==sizeof hdr+size_t(hdr.sampleCount)*sizeof(int16_t) && tracks[i].pcm;
    bool ok=headerOK;
    if(ok){
      const size_t bytes=size_t(hdr.sampleCount)*sizeof(int16_t);
      ok=f.read(reinterpret_cast<uint8_t*>(tracks[i].pcm),bytes)==bytes;
      if(ok)ok=clipChecksum(tracks[i].pcm,hdr.sampleCount)==hdr.checksum;
    }
    tracks[i].length=ok?hdr.sampleCount:0;f.close();
    Serial.printf("Load track %d: %s\n",i+1,ok?"OK":"invalid / skipped");
  }
}

int16_t *samples=nullptr,*playChunks[kPlayBuffers]={};
size_t captured=0;
void commitActiveTrack(){tracks[activeTrack].length=captured;}
bool selectTrack(int which);
uint8_t waveform[kWaveBins]={};
float spectrum[kBands]={};
M5Canvas canvas(&M5.Display);
Preferences prefs;
bool canvasReady=false,scrollConnected=false,scrollDown=false,scrollShort=false,scrollLong=false,scrollLongHandled=false;
int32_t wheelLast=0;
uint32_t lastPoll=0,lastProbe=0,lastFrame=0,pressedAt=0,lastBatteryPoll=0;
uint8_t volumePct=65,brightnessPct=80,batteryPct=0;
enum class LoopMode : uint8_t { Off=0, One=1, All=2 };
LoopMode loopMode=LoopMode::Off;
void cycleLoopMode(){loopMode=static_cast<LoopMode>((static_cast<uint8_t>(loopMode)+1)%3);}
bool charging=false,settingsEditing=false,chargeScreenOff=false;
uint32_t chargeLastInteraction=0;
bool aWasDown=false,aHoldStarted=false;uint32_t aPressedAt=0;
int homeSelection=0,settingsSelection=0;
uint8_t themeSelection=0; // 0=GREEN, 1=RED, 2=BLUE
float playbackSeconds=0.0f;
// Position in the currently selected clip. 0=all tape on supply, 1=all on take-up.
float tapePosition=0.0f;
void persistActiveTrack(){prefs.putUChar("active",uint8_t(activeTrack));}
float reelWheelNudge=0.0f; // transient reel rotation from idle wheel movement
uint32_t reelWheelNudgeAt=0;
uint32_t limitFlashUntil=0;
enum class Page{Home,Chomp,Tracks,Settings,Effects,HowTo,About,Charge};
enum class State{Idle,Recording,Playing,Error};
Page page=Page::Chomp;
State state=State::Idle;
const char* errorText="";
constexpr uint16_t BG=0x0841,PANEL=0x18C3,TRIM=0x6B4D,GREEN=0x07E0,LIME=0xBFE0,PURPLE=0xB1BC,YELLOW=0xFFE0,RED=0xF800,MUTED=0xA534,WHITE=0xE71C;
// Four logical RGB565 theme roles. The secondary is used for reverse mode.
struct UiTheme {uint16_t primary,secondary,warning,active;const char* name;};
constexpr uint16_t rgb565(uint8_t r,uint8_t g,uint8_t b){return uint16_t((r>>3)<<11)|uint16_t((g>>2)<<5)|uint16_t(b>>3);}
static constexpr UiTheme kThemes[]={
  {rgb565(0,255,0),rgb565(255,0,255),rgb565(255,255,0),rgb565(255,153,0),"GREEN"},
  {rgb565(255,0,0),rgb565(0,255,174),rgb565(255,255,0),rgb565(255,153,0),"RED"},
  {rgb565(0,255,247),rgb565(255,0,68),rgb565(255,255,0),rgb565(255,153,0),"BLUE"},
};
const UiTheme& palette(){return kThemes[constrain(int(themeSelection),0,2)];}
uint16_t themeAccent(){return palette().primary;}
uint16_t accent(){return speed()<0?palette().secondary:palette().primary;}
bool selectTrack(int which){
  if(which<0||which>=kTrackCount)return false;
  if(activeTrack==which)return true;
  commitActiveTrack();
  activeTrack=which;samples=tracks[which].pcm;captured=tracks[which].length;
  tapePosition=0.0f;persistActiveTrack();
  return samples!=nullptr;
}


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
// Canonical sprite drawing: exact image pixels, guide borders excluded at export.
// Transparent=skip, opaque #FF00FF=theme primary, #00FFFF=secondary.
void drawAtlasSprite(const ChompArt::Sprite& sprite,int x,int y,uint16_t primary,uint16_t secondary=0){
  if(!secondary)secondary=palette().secondary;
  for(int py=0;py<sprite.h;++py){
    for(int px=0;px<sprite.w;++px){
      const uint32_t c=sprite.pixels[py*sprite.w+px];
      if(c==0)continue;
      const uint16_t out=c==0x10000?primary:(c==0x20000?secondary:uint16_t(c&0xffff));
      canvas.drawPixel(x+px,y+py,out);
    }
  }
}
void drawTeeth(int x,int y){
  drawAtlasSprite(ChompBrand::Teeth,x,y,themeAccent());
}
void drawHeader(const char* title){
  int w=canvas.width();
  canvas.fillRect(2,2,w-4,17,PANEL);
  canvas.setTextSize(1);canvas.setTextColor(themeAccent(),PANEL);canvas.setCursor(24,7);canvas.print("SOUND-CHOMP");
  drawTeeth(7,5);
  int batteryX=w-22;
  canvas.drawRect(batteryX,7,15,8,themeAccent());
  canvas.fillRect(batteryX-3,9,3,4,themeAccent()); // battery terminal on left
  canvas.fillRect(batteryX+2,9,constrain((int(batteryPct)*11)/100,0,11),4,batteryPct<20?palette().warning:themeAccent());
  // Portrait title needs the entire width; omit battery percentage there.
  if(canvas.width()>canvas.height()){
    canvas.setTextColor(themeAccent(),PANEL);canvas.setCursor(batteryX-31,7);
    canvas.printf("%u%%",unsigned(batteryPct));
  }
  // Charging shown by percentage/status; avoid an orphan contrasting pixel within the fill.
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
      uint16_t color=frac>0.89f?palette().warning:(frac>0.72f?palette().active:accent());
      canvas.fillRect(x,top+height-2-yy,stride-2,min(3,h-yy),color);
    }
  }
}
void drawDirectionIcon(int x,int y,bool reverse){
  // A pair of unmistakable VCR-style chevrons: << or >>.
  // Open interior makes the silhouettes read as arrows rather than diamonds.
  const uint16_t c=accent();
  for(int part=0;part<2;part++){
    int ox=x+part*9;
    for(int k=0;k<4;k++){
      int px=reverse?ox+3-k:ox+k;
      canvas.drawFastHLine(px,y+k,2,c);
      canvas.drawFastHLine(px,y+6-k,2,c);
      // One-pixel highlight on each flank preserves visibility on the LCD.
      if(k==3)canvas.drawPixel(px,y+3,c);
    }
  }
}
void drawTapeReels(int w){
  // Both cassette hubs turn counter-clockwise in forward play, clockwise in
  // reverse. Colored tape rings transfer from left/supply to right/take-up.
  const bool portrait=canvas.height()>canvas.width();
  const int cy=portrait?75:41;
  const int centers[2]={portrait?w/2-21:w-66,portrait?w/2+21:w-27};
  const uint32_t now=millis();
  float transport=state==State::Recording?1.0f:(state==State::Playing?speed():0.0f);
  if(state==State::Idle && reelWheelNudgeAt && now-reelWheelNudgeAt<320)
    transport=reelWheelNudge;
  static float angle=0.0f;
  static uint32_t previous=0;
  if(previous){
    const uint32_t dt=min(uint32_t(100),now-previous);
    // Negative angle is counter-clockwise in screen coordinates.
    angle-=transport*(float(dt)*0.0042f);
  }
  previous=now;
  const float p=constrain(tapePosition,0.0f,1.0f);
  for(int j=0;j<2;j++){
    const int cx=centers[j];
    const uint16_t tint=(now<limitFlashUntil)?palette().warning:accent();
    // Hub radius 11, with a 2..5px band of magnetic tape outside.
    // A full supply reel becomes empty as the take-up reel fills.
    const float fill=j==0?1.0f-p:p;
    const int outer=13+int(lroundf(fill*3.0f));
    canvas.fillCircle(cx,cy,outer,tint);
    canvas.fillCircle(cx,cy,11,PANEL);
    canvas.drawCircle(cx,cy,11,WHITE);
    for(int k=0;k<3;k++){
      float a=angle+float(k)*2.094395f+float(j)*0.45f;
      int x0=cx+int(cosf(a)*4),y0=cy+int(sinf(a)*4);
      int x1=cx+int(cosf(a)*9),y1=cy+int(sinf(a)*9);
      canvas.drawLine(x0,y0,x1,y1,tint);
      canvas.drawLine(x0+1,y0,x1+1,y1,tint);
    }
    // Black center remains opaque over the recording banner.
    canvas.fillCircle(cx,cy,3,TFT_BLACK);
    canvas.drawPixel(cx,cy,WHITE);
  }
}

void drawLoopIcon(int x,int y){
  const bool enabled=loopMode!=LoopMode::Off;
  uint16_t c=enabled?themeAccent():MUTED;
  canvas.drawFastHLine(x+3,y,10,c);canvas.drawFastVLine(x+14,y+1,7,c);
  canvas.drawFastHLine(x+3,y+8,11,c);canvas.drawFastVLine(x+2,y+1,6,c);
  canvas.fillTriangle(x+11,y-2,x+16,y,x+11,y+2,c);
  canvas.fillRect(x+19,y+3,3,3,enabled?palette().active:0x2131);
  // ON: 1 or A (all). OFF: unlit indicator. Keep transport-row footprint.
  if(enabled){
    canvas.setTextSize(1);canvas.setTextColor(themeAccent(),BG);
    canvas.setCursor(x+24,y+1);
    if(loopMode==LoopMode::One)canvas.print("1");
    else canvas.printf("ALL(%d)",activeTrack+1);
  }
}
const ChompArt::Sprite& timerSprite(char c){
 switch(c){
 case '0':return ChompArt::Digit0;case '1':return ChompArt::Digit1;case '2':return ChompArt::Digit2;
 case '3':return ChompArt::Digit3;case '4':return ChompArt::Digit4;case '5':return ChompArt::Digit5;
 case '6':return ChompArt::Digit6;case '7':return ChompArt::Digit7;case '8':return ChompArt::Digit8;
 case '9':return ChompArt::Digit9;case '.':return ChompArt::SymDot;default:return ChompArt::SymS;
 }
}
int timeWidth(const char* str){int total=0;for(const char* t=str;*t;++t)total+=timerSprite(*t).w+1;return total?total-1:0;}
void drawTimeLabel(int right,int y,const char* str){
 int x=right-timeWidth(str);
 for(const char* t=str;*t;++t){
  const auto& glyph=timerSprite(*t);
  drawAtlasSprite(glyph,x,y+(12-int(glyph.h))/2,state==State::Playing?palette().active:palette().warning);
  x+=glyph.w+1;
 }
}
void drawWaveform(int left,int top,int width,int height){
  const int cy=top+height/2;
  canvas.drawFastHLine(left,cy,width,TRIM);
  if(captured==0){
    canvas.setTextSize(1);canvas.setTextColor(MUTED,PANEL);
    const char* prompt="TAP A: PLAY  HOLD A: REC";
    canvas.setCursor(left+max(2,(width-int(strlen(prompt))*6)/2),cy-4);
    canvas.print(prompt);return;
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
    if(marker>=0&&marker<width)canvas.drawFastVLine(left+marker,top,height,palette().warning);
  }
}
// A compact two-tier, six-button transport strip; all geometry fits 240x135.
void drawTransportButtons(int w,int h){
  const bool portrait=h>w;
  const int columns=portrait?3:6;
  const int x0=4,gap=portrait?2:1;
  // Landscape: six buttons use all available width, timer occupies far right.
  const int timerWidth=32,timerGap=1;
  const int area=portrait?w-2*x0:w-2*x0-timerWidth-timerGap;
  const int bw=(area-(columns-1)*gap)/columns;
  const int bh=portrait?27:27;
  const int y0=portrait?h-65:h-30;
  for(int i=0;i<6;i++){
    const int col=i%columns,row=i/columns;
    const int x=x0+col*(bw+gap),y=y0+row*(bh+gap);
    const bool selected=int(playerFocus)==i;
    const uint16_t border=selected?(playerEditing?palette().active:themeAccent()):TRIM;
    const uint16_t c=selected?themeAccent():WHITE;
    // Do not flood-fill the button; native RGB565 artwork shows through.
    // Editable atlas frame underneath the live focus border and glyphs.
    const auto& frame=portrait?ChompArt::BtnPStd:(i==0?ChompArt::BtnLPlay:i==1?ChompArt::BtnLSpeed:
      i==2?ChompArt::BtnLFx1:i==3?ChompArt::BtnLFx2:i==4?ChompArt::BtnLFx3:ChompArt::BtnLLoop);
    // Source frames remain pixel-perfect (no scaling); frame origin follows slot.
    if(frame.w<=bw && frame.h<=bh)drawAtlasSprite(frame,x,y,themeAccent());
    canvas.drawRect(x,y,bw,bh,border);
    canvas.drawFastHLine(x+1,y+1,bw-2,selected?WHITE:MUTED);
    canvas.fillRect(x+bw-5,y+4,3,3,selected?(playerEditing?palette().active:themeAccent()):0x10A2);
    const int cx=x+bw/2;
    if(i==0){
      if(state==State::Playing)drawAtlasSprite(ChompArt::IconPause,cx-6,y+6,c);
      else drawAtlasSprite(ChompArt::IconPlay,cx-7,y+6,c);
    }else if(i==1){
      drawAtlasSprite(ChompArt::IconSpeed,cx-8,y+5,c);
      char sp[12];snprintf(sp,sizeof(sp),"%+.1fx",double(speed()));
      canvas.setTextColor(c);canvas.setTextSize(1);
      canvas.setCursor(cx-int(strlen(sp))*3,y+17);canvas.print(sp);
    }else if(i>=2&&i<=4){
      const ChompArt::Sprite& fx=i==2?ChompArt::IconFx1Echo:i==3?ChompArt::IconFx2Stutter:ChompArt::IconFx3Robot;
      drawAtlasSprite(fx,cx-fx.w/2,y+7,c);
      if((i==3&&stutterEnabled)||(i==4&&robotEnabled))canvas.fillRect(x+3,y+3,3,3,palette().active);
      if(i==2&&echoEnabled)canvas.fillRect(x+3,y+3,3,3,palette().active);
    }else{
      const ChompArt::Sprite& li=loopMode==LoopMode::Off?ChompArt::IconLoopOff:(loopMode==LoopMode::One?ChompArt::IconLoopOne:ChompArt::IconLoopAll);
      drawAtlasSprite(li,cx-8,y+4,c);
      const char* mode=loopMode==LoopMode::Off?"OFF":loopMode==LoopMode::One?"1":"ALL";
      canvas.setTextColor(c);canvas.setTextSize(1);
      canvas.setCursor(cx-int(strlen(mode))*3,y+17);canvas.print(mode);
    }
  }
  char t[16];snprintf(t,sizeof(t),"%.1fs",double(state==State::Playing?playbackSeconds:captured/double(kSampleRate)));
  if(!portrait){
    const int tx=w-x0-timerWidth;
    // Restore custom seven-segment clock with centered text and 1px spacing.
    const int tw=timeWidth(t);
    drawTimeLabel(tx+(timerWidth+tw)/2,y0+(bh-12)/2,t);
  }else{
    canvas.setTextSize(1);canvas.setTextColor(themeAccent());
    canvas.setCursor(w-int(strlen(t))*6-7,y0-13);canvas.print(t);
  }
}


// Single non-wrapping transport/status strip. The speed number remains in the
// lower control button; only direction and enabled indicators appear here.
void drawCompactStatus(int areaLeft,int areaWidth,int y,bool center){
  char text[50];
  int used=snprintf(text,sizeof(text),"TRK:%d",activeTrack+1);
  if(echoEnabled && used>=0 && used<int(sizeof(text)))used+=snprintf(text+used,sizeof(text)-used," E1");
  if(stutterEnabled && used>=0 && used<int(sizeof(text)))used+=snprintf(text+used,sizeof(text)-used," E2");
  if(robotEnabled && used>=0 && used<int(sizeof(text)))used+=snprintf(text+used,sizeof(text)-used," E3");
  if(loopMode!=LoopMode::Off && used>=0 && used<int(sizeof(text)))
    snprintf(text+used,sizeof(text)-used," %s",loopMode==LoopMode::One?"L1":"ALL");
  canvas.setTextSize(1);
  const int fullWidth=19+5+int(strlen(text))*6;
  int x=center?areaLeft+(areaWidth-fullWidth)/2:areaLeft;
  x=max(areaLeft,x);
  drawDirectionIcon(x,y+1,speed()<0);
  canvas.setTextColor(WHITE);
  canvas.setCursor(x+24,y);canvas.print(text);
}

void drawChomp(){
  const int w=canvas.width(),h=canvas.height();
  const bool portrait=h>w;
  if(portrait){
    // Independent vertical composition; leave enough space for 2x3 controls.
    if(state==State::Recording){
      canvas.fillRect(5,23,w-10,36,RED);
      const int dotR=4;
      const int recWidth=3*12;
      const int combined=dotR*2+6+recWidth;
      const int x=(w-combined)/2;
      if((millis()/250)%2==0)canvas.fillCircle(x+dotR,37,dotR,TFT_BLACK);
      canvas.setTextColor(TFT_BLACK);canvas.setTextSize(2);
      canvas.setCursor(x+dotR*2+6,29);canvas.print("REC");
    }else{
      const char* heading=state==State::Playing?"PLAYING":state==State::Error?"ERROR":"READY";
      canvas.setTextSize(2);canvas.setTextColor(accent());
      const int titleX=(w-canvas.textWidth(heading))/2;
      canvas.setCursor(titleX,28);canvas.print(heading);
      drawCompactStatus(4,w-8,51,true);
    }
    // Reels lowered for vertical arrangement, using existing art function.
    drawTapeReels(w);
    const int left=8,top=97,vw=w-16,vh=65;
    canvas.drawRect(left-2,top-3,vw+4,vh+6,TRIM);
    canvas.fillRect(left,top,vw,vh,PANEL);
    if(state==State::Playing||state==State::Recording)drawSpectrum(left,top,vw,vh);
    else drawWaveform(left,top,vw,vh);
  }else{
    if(state==State::Recording){
      canvas.fillRect(5,22,w-10,39,RED);
      if((millis()/250)%2==0)canvas.fillCircle(17,36,4,TFT_BLACK);
      canvas.setTextSize(2);canvas.setTextColor(TFT_BLACK,RED);
      canvas.setCursor(29,29);canvas.print("RECORDING");
    }else{
      const char* heading=state==State::Playing?"PLAYING":state==State::Error?"ERROR":"READY";
      canvas.setTextSize(2);canvas.setTextColor(accent());canvas.setCursor(8,25);canvas.print(heading);
      if(playerEditing){
        canvas.setTextSize(1);canvas.setTextColor(WHITE);canvas.setCursor(9,49);
        switch(playerFocus){
          case PlayerFocus::Speed:canvas.printf("SPD %+.1fx",double(speed()));break;
          case PlayerFocus::EchoToggle:canvas.printf("FX1 %s %ums",echoEnabled?"ON":"OFF",unsigned(echoDelayMs));break;
          case PlayerFocus::Stutter:canvas.printf("FX2 %s %ums",stutterEnabled?"ON":"OFF",unsigned(stutterSliceMs));break;
          case PlayerFocus::Robot:canvas.printf("FX3 %s %u%%",robotEnabled?"ON":"OFF",unsigned(robotIntensity));break;
          case PlayerFocus::Loop:canvas.printf("LOOP %s",loopMode==LoopMode::Off?"OFF":loopMode==LoopMode::One?"1":"ALL");break;
          default:drawCompactStatus(9,132,49,false);break;
        }
      }else drawCompactStatus(9,132,49,false);
    }
    drawTapeReels(w);
    const int left=8,top=65,vw=w-16,vh=max(12,h-103);
    canvas.drawRect(left-2,top-3,vw+4,vh+6,TRIM);
    canvas.fillRect(left,top,vw,vh,PANEL);
    if(state==State::Playing||state==State::Recording)drawSpectrum(left,top,vw,vh);
    else drawWaveform(left,top,vw,vh);
  }
  drawTransportButtons(w,h);
  if(state==State::Error){canvas.setTextColor(RED,BG);canvas.setTextSize(1);canvas.setCursor(8,60);canvas.print(errorText);}
}

void drawMenuIcon(int x,int y,int index,uint16_t color){
  // Small, original 9x9 pixel symbols; distinguishable without reading.
  switch(index){
    case 0:canvas.fillCircle(x+4,y+4,3,color);canvas.fillCircle(x+4,y+4,1,BG);break; // record
    case 1:for(int i=0;i<4;i++)canvas.drawFastVLine(x+i+2,y+2+i,7-2*i,color);break; // tracks/play
    case 2:canvas.drawCircle(x+4,y+4,3,color);canvas.drawFastHLine(x+2,y+4,5,color);break;
    case 3:canvas.drawFastVLine(x+2,y+4,4,color);canvas.drawFastVLine(x+5,y+1,7,color);canvas.drawFastVLine(x+8,y+3,5,color);break;
    case 4:canvas.drawRect(x+1,y+1,7,7,color);canvas.drawPixel(x+4,y+3,color);canvas.drawPixel(x+4,y+5,color);break;
    case 5:canvas.drawCircle(x+4,y+4,3,color);canvas.drawFastVLine(x+4,y+3,4,color);break;
    default:canvas.drawRect(x+1,y+2,7,5,color);canvas.fillRect(x,y+4,1,2,color);break;
  }
}
void drawHome(){
  int w=canvas.width(),h=canvas.height();
  const char* items[]={"CHOMP","TRACKS","EFFECTS","SETTINGS","HOW TO","ABOUT","CHARGE"};
  // Landscape: two columns. Portrait: one readable column.
  const int perColumn=h>w?7:4, gap=5, margin=7;
  const int cw=h>w?w-2*margin:(w-2*margin-gap)/2;
  for(int i=0;i<7;i++){
    int x=margin+(i/perColumn)*(cw+gap);
    int y=24+(i%perColumn)*(h>w?28:24);
    bool selected=i==homeSelection;
    uint16_t surface=PANEL;
    canvas.fillRect(x,y,cw,21,surface);
    canvas.drawRect(x,y,cw,21,selected?themeAccent():TRIM);
    drawMenuIcon(x+6,y+6,i,selected?themeAccent():WHITE);
    canvas.setTextSize(1);canvas.setTextColor(selected?themeAccent():WHITE,surface);
    canvas.setCursor(x+20,y+7);canvas.print(items[i]);
  }
}
void drawTracks(){
  int w=canvas.width(),h=canvas.height();
  for(int i=0;i<kTrackCount;i++){
    int y=23+i*22;
    uint16_t surface=PANEL;
    canvas.fillRect(7,y,w-14,20,surface);
    canvas.drawRect(7,y,w-14,20,i==trackSelection?themeAccent():TRIM);
    canvas.setTextSize(1);canvas.setTextColor(i==trackSelection?themeAccent():WHITE,surface);
    canvas.setCursor(13,y+6);canvas.printf("%c CHOMP %02d",i==activeTrack?'*':' ',i+1);
    canvas.setCursor(w-88,y+6);
    if(tracks[i].length)canvas.printf("%.1fs",double(tracks[i].length)/kSampleRate);
    else canvas.print("EMPTY");
  }
  canvas.setTextSize(1);canvas.setTextColor(trackDeleteConfirm?RED:WHITE,BG);
  canvas.setCursor(8,h-17);
  if(trackDeleteConfirm)canvas.print("CLICK: DELETE  B: CANCEL");
  else canvas.print("A/CLICK: OPEN  HOLD: CLEAR");
}
void drawSettings(){
  int w=canvas.width(),h=canvas.height();
  const char* labels[]={"VOLUME","BRIGHT","REVERSE MAP","AUTO PLAY","RESET SPEED","ORIENTATION","THEME"};
  const char* shortLabels[]={"VOL","BRGT","REV","AUTO","RESET","ORIENT","THEME"};
  for(int i=0;i<7;i++){
    int y=19+i*14;bool sel=settingsSelection==i;
    uint16_t surface=PANEL;
    canvas.fillRect(7,y,w-14,14,surface);
    canvas.drawRect(7,y,w-14,14,sel?(settingsEditing?palette().active:themeAccent()):TRIM);
    canvas.setTextColor(sel?themeAccent():WHITE,surface);canvas.setTextSize(1);
    canvas.setCursor(13,y+3);canvas.print(h>w?shortLabels[i]:labels[i]);
    canvas.setCursor(w-(h>w?52:75),y+3);
    if(i==0)canvas.printf("%u%%",unsigned(volumePct));
    else if(i==1)canvas.printf("%u%%",unsigned(brightnessPct));
    else if(i==2)canvas.print(invertedReverse?"INVERTED":"NORMAL");
    else if(i==3)canvas.print(autoPlay?"ON":"OFF");
    else if(i==4)canvas.print(resetSpeedAfterRecord?"ON":"OFF");
    else if(i==5)canvas.print(orientationMode==OrientationMode::Landscape?"LAND":orientationMode==OrientationMode::Portrait?"VERT":"AUTO");
    else canvas.print(palette().name);
  }
  canvas.setTextColor(WHITE,BG);canvas.setCursor(7,h-14);
  canvas.print(settingsEditing?"TURN:CHANGE CLICK:DONE":"TURN:SELECT CLICK:EDIT");
}

void drawInfoPage(bool about){
  canvas.setTextSize(1);canvas.setTextColor(themeAccent(),BG);
  if(about){
    canvas.setCursor(8,29);canvas.print("SOUND-CHOMP ");canvas.print(kFirmwareVersion);
    canvas.setTextColor(WHITE,BG);canvas.setCursor(8,47);canvas.print("BUILT BY u/eyeofbri");
    canvas.setCursor(8,65);canvas.print("BUILT FOR AIDEN");
    canvas.setCursor(8,82);canvas.print("github.com/eyeofbri/");
    canvas.setCursor(8,92);canvas.print("Sound-Chomp");
  }else{
    const char* keys[]={"TAP A:","HOLD A:","RELEASE:","WHEEL:","CLICK:","HOLD CLICK:","B:"};
    const char* descriptions[]={"PLAY / STOP","RECORD","STOP RECORDING","NAV / ADJUST","EDIT / PLAY","FOCUS PLAY","MENU"};
    for(int i=0;i<7;i++){
      int y=24+i*14;
      canvas.setTextColor(themeAccent(),BG);canvas.setCursor(8,y);canvas.print(keys[i]);
      canvas.setTextColor(WHITE,BG);canvas.setCursor(88,y);canvas.print(descriptions[i]);
    }
  }
  canvas.setTextColor(MUTED,BG);canvas.setCursor(8,canvas.height()-13);canvas.print("B: MENU");
}
void drawCharge(){
  const int w=canvas.width();
  const int bx=44,by=35,bw=w-88,bh=56;
  canvas.drawRoundRect(bx,by,bw,bh,6,themeAccent());
  canvas.fillRect(bx-8,by+17,8,22,themeAccent()); // same left-side terminal
  const int fill=(bw-10)*batteryPct/100;
  canvas.fillRoundRect(bx+5,by+5,fill,bh-10,3,batteryPct<20?palette().warning:themeAccent());
  canvas.setTextSize(2);canvas.setTextColor(themeAccent(),BG);
  char buf[12];snprintf(buf,sizeof buf,"%u%%",unsigned(batteryPct));
  canvas.setCursor((w-int(strlen(buf))*12)/2,101);canvas.print(buf);
  if(charging){canvas.setTextSize(1);canvas.setCursor(92,23);canvas.print("CHARGING");}
}
void drawEffects(){
  const int w=canvas.width(),h=canvas.height();
  const bool portrait=h>w;
  canvas.setTextSize(1);canvas.setTextColor(themeAccent());
  canvas.setCursor(7,22);canvas.print("EFFECTS");
  const int rowY[3]={37,62,87};
  const char* labels[3]={"ECHO", "STUTTER", "ROBOT"};
  for(int row=0;row<3;++row){
    const int y=rowY[row];
    const bool focus=effectsSelection==row;
    canvas.drawRect(5,y,w-10,22,focus?themeAccent():TRIM);
    canvas.setTextColor(focus?palette().active:WHITE);canvas.setCursor(9,y+7);
    canvas.print(labels[row]);
    const bool enabled=row==0?echoEnabled:row==1?stutterEnabled:robotEnabled;
    // In portrait show one selected row's values in a separate detail line.
    if(!portrait){
      const int x=w-132;
      canvas.setTextColor((focus&&effectsEditing&&effectsField==0)?palette().active:WHITE);
      canvas.setCursor(x,y+7);canvas.print(enabled?"ON":"OFF");
      canvas.setTextColor((focus&&effectsEditing&&effectsField==1)?palette().active:WHITE);
      canvas.setCursor(x+39,y+7);
      if(row==0)canvas.printf("%u%%",unsigned(echoWet));
      else if(row==1)canvas.printf("%ums",unsigned(stutterSliceMs));
      else canvas.printf("%u%%",unsigned(robotIntensity));
      if(row==0){
        canvas.setTextColor((focus&&effectsEditing&&effectsField==2)?palette().active:WHITE);
        canvas.setCursor(x+79,y+7);canvas.printf("%ums",unsigned(echoDelayMs));
      }
    }else{
      canvas.setTextColor(WHITE);canvas.setCursor(w-34,y+7);canvas.print(enabled?"ON":"OFF");
    }
  }
  if(portrait){
    canvas.setTextColor(themeAccent());canvas.setCursor(8,120);
    if(effectsSelection==0)canvas.printf("MIX %u%%  DEL %ums",unsigned(echoWet),unsigned(echoDelayMs));
    else if(effectsSelection==1)canvas.printf("SLICE %ums",unsigned(stutterSliceMs));
    else canvas.printf("LEVEL %u%%",unsigned(robotIntensity));
  }
  canvas.setTextColor(MUTED);canvas.setCursor(8,h-13);
  canvas.print(effectsEditing?"TURN: EDIT CLICK: NEXT":"TURN: SELECT CLICK: EDIT");
}

void draw(bool force=false){
  if(!canvasReady)return;
  if(page==Page::Charge&&chargeScreenOff){updateBattery();return;}
  if(!force&&millis()-lastFrame<kFrameMs)return;
  lastFrame=millis();updateBattery();
  // Static pre-rendered RGB565 art, followed by dynamic off-screen rendering.
  if(page==Page::Chomp){
    if(canvas.width()>canvas.height()){
      const uint16_t* bg=themeSelection==1?ChompArt::kRetroLandscapeRed:
                         themeSelection==2?ChompArt::kRetroLandscapeBlue:ChompArt::kRetroLandscapeGreen;
      canvas.pushImage(0,0,240,135,bg);
    }else{
      const uint16_t* bg=themeSelection==1?ChompArt::kRetroPortraitRed:
                         themeSelection==2?ChompArt::kRetroPortraitBlue:ChompArt::kRetroPortraitGreen;
      canvas.pushImage(0,0,135,240,bg);
    }
  }else canvas.fillSprite(BG);
  drawHeader(page==Page::Home?"SOUND-CHOMP":page==Page::Chomp?"CHOMP / PLAYER":page==Page::Tracks?"TRACKS":page==Page::Effects?"EFFECTS":page==Page::HowTo?"HOW TO":page==Page::About?"ABOUT":page==Page::Charge?"CHARGE":"SETTINGS");
  if(page==Page::Home)drawHome();else if(page==Page::Chomp)drawChomp();
  else if(page==Page::Tracks)drawTracks();else if(page==Page::Settings)drawSettings();
  else if(page==Page::Effects)drawEffects();else if(page==Page::Charge)drawCharge();else drawInfoPage(page==Page::About);
  canvas.pushSprite(0,0);
}
void saveEchoSettings();
void markEchoSettingsDirty(){effectPreferenceDirty=true;effectPreferenceChangedAt=millis();}
void handleWheel(int delta){
  if(page==Page::Home)homeSelection=constrain(homeSelection+delta,0,6);
  else if(page==Page::Tracks){trackSelection=constrain(trackSelection+delta,0,kTrackCount-1);trackDeleteConfirm=false;}
  else if(page==Page::Chomp){
    if(!playerEditing){
      playerFocus=static_cast<PlayerFocus>(constrain(int(playerFocus)+delta,0,5));
    }else if(playerFocus==PlayerFocus::Speed){
      const int before=speedIndex;
      speedIndex=constrain(speedIndex+delta,0,kSpeedCount-1);
      if(state==State::Idle){
        if(speedIndex==before){reelWheelNudge=0;reelWheelNudgeAt=0;limitFlashUntil=millis()+220;}
        else {reelWheelNudge=delta>0?1.7f:-1.7f;reelWheelNudgeAt=millis();tapePosition=constrain(tapePosition+0.035f*float(delta),0.0f,1.0f);}
      }
    }else if(playerFocus==PlayerFocus::EchoToggle){
      // Off at zero; forward wheel positions enable echo and set delay.
      if(delta<0 && echoDelayMs<=kEchoMinMs)echoEnabled=false;
      else if(delta>0 && !echoEnabled){echoEnabled=true;echoDelayMs=kEchoMinMs;}
      else if(echoEnabled)echoDelayMs=uint16_t(constrain(int(echoDelayMs)+delta*25,kEchoMinMs,kEchoMaxMs));
      markEchoSettingsDirty();
    }else if(playerFocus==PlayerFocus::Stutter){
      if(delta<0 && stutterSliceMs<=80)stutterEnabled=false;
      else if(delta>0 && !stutterEnabled){stutterEnabled=true;stutterSliceMs=80;}
      else if(stutterEnabled)stutterSliceMs=uint16_t(constrain(int(stutterSliceMs)+delta*20,80,400));
      markEchoSettingsDirty();
    }else if(playerFocus==PlayerFocus::Robot){
      if(delta<0 && robotIntensity<=5)robotEnabled=false;
      else if(delta>0 && !robotEnabled){robotEnabled=true;robotIntensity=5;}
      else if(robotEnabled)robotIntensity=uint8_t(constrain(int(robotIntensity)+delta*5,5,100));
      markEchoSettingsDirty();
    }else if(playerFocus==PlayerFocus::Loop){
      loopMode=static_cast<LoopMode>(constrain(int(loopMode)+delta,0,2));
    }
  }else if(page==Page::Effects){
    if(!effectsEditing)effectsSelection=constrain(effectsSelection+delta,0,2);
    else if(effectsSelection==0){
      if(effectsField==0&&delta)echoEnabled=delta>0;
      else if(effectsField==1)echoWet=uint8_t(constrain(int(echoWet)+delta*5,0,100));
      else if(effectsField==2)echoDelayMs=uint16_t(constrain(int(echoDelayMs)+delta*kEchoStepMs,kEchoMinMs,kEchoMaxMs));
      markEchoSettingsDirty();
    }else if(effectsSelection==1){
      if(effectsField==0&&delta)stutterEnabled=delta>0;
      else if(effectsField==1)stutterSliceMs=uint16_t(constrain(int(stutterSliceMs)+delta*20,80,400));
      markEchoSettingsDirty();
    }else{
      if(effectsField==0&&delta)robotEnabled=delta>0;
      else if(effectsField==1)robotIntensity=uint8_t(constrain(int(robotIntensity)+delta*5,0,100));
      markEchoSettingsDirty();
    }
  }else if(page==Page::Settings){
    if(settingsEditing){
      if(settingsSelection==0){volumePct=uint8_t(constrain(int(volumePct)+delta*5,0,100));setVolume();}
      else if(settingsSelection==1){brightnessPct=uint8_t(constrain(int(brightnessPct)+delta*5,5,100));M5.Display.setBrightness(uint8_t(int(brightnessPct)*255/100));}
      else if(settingsSelection==2 && delta!=0)invertedReverse=delta>0;
      else if(settingsSelection==3 && delta!=0)autoPlay=delta>0;
      else if(settingsSelection==4 && delta!=0)resetSpeedAfterRecord=delta>0;
      else if(settingsSelection==5 && delta!=0)orientationMode=static_cast<OrientationMode>(constrain(int(orientationMode)+(delta>0?1:-1),0,2));
      else if(settingsSelection==6 && delta!=0)themeSelection=uint8_t(constrain(int(themeSelection)+delta,0,2));
    }else settingsSelection=constrain(settingsSelection+delta,0,6);
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
void saveSettings(){prefs.putUChar("volume",volumePct);prefs.putUChar("bright",brightnessPct);prefs.putBool("revInvert",invertedReverse);prefs.putBool("autoPlay",autoPlay);prefs.putBool("resetSpd",resetSpeedAfterRecord);prefs.putUChar("orientation",uint8_t(orientationMode));prefs.putUChar("theme",themeSelection);}
void saveEchoSettings(){
  prefs.putBool("echoOn",echoEnabled);prefs.putUChar("echoWet",echoWet);prefs.putUShort("echoDelay",echoDelayMs);
  prefs.putBool("stutOn",stutterEnabled);prefs.putUShort("stutMs",stutterSliceMs);
  prefs.putBool("robotOn",robotEnabled);prefs.putUChar("robotMix",robotIntensity);
}
void backHome(){trackDeleteConfirm=false;aWasDown=false;aHoldStarted=false;if(page==Page::Settings)saveSettings();if(page==Page::Effects)saveEchoSettings();settingsEditing=false;effectsEditing=false;page=Page::Home;clearEvents();draw(true);}
void openSelection(){
  aWasDown=M5.BtnA.isPressed();aHoldStarted=true;
  page=homeSelection==0?Page::Chomp:homeSelection==1?Page::Tracks:
       homeSelection==2?Page::Effects:homeSelection==3?Page::Settings:
       homeSelection==4?Page::HowTo:homeSelection==5?Page::About:Page::Charge;
  if(page==Page::Charge){chargeScreenOff=false;chargeLastInteraction=millis();M5.Display.setBrightness(uint8_t(int(brightnessPct)*255/100));}
  clearEvents();draw(true);
}
void recordClip(){
  if(!samples)return;
  captured=0;tracks[activeTrack].length=0;tapePosition=0.0f;M5.Speaker.end();
  if(!M5.Mic.begin()){state=State::Error;errorText="MIC BEGIN FAILED";return;}
  state=State::Recording;setStateLed();clearEvents();draw(true);
  while(captured<kMaxSamples){
    if(!M5.BtnA.isPressed()||M5.BtnB.wasPressed())break;
    size_t count=min(kRecordChunk,kMaxSamples-captured);
    if(!M5.Mic.record(samples+captured,count,kSampleRate)){
      state=State::Error;errorText="MIC RECORD FAILED";break;
    }
    while(M5.Mic.isRecording()){service();draw();delay(1);}
    captured+=count;tapePosition=float(captured)/float(kMaxSamples);analyze(samples,captured,captured-count/2);service();draw();
  }
  M5.Mic.end();commitActiveTrack();updateWaveform();if(resetSpeedAfterRecord)resetSpeed();if(state!=State::Error)state=State::Idle;
  if(state==State::Idle){
    if(storageReady){storageSaveFailed=!saveTrack(activeTrack);if(storageSaveFailed)Serial.println("WARNING: Clip only in RAM; flash save failed");}
    else Serial.println("WARNING: flash not mounted; recording only in RAM");
  }
  setStateLed();clearEvents();draw(true);
  Serial.printf("Captured %.2fs\n",double(captured)/kSampleRate);
}
// Generate output from the original PCM at the selected signed speed.
// IMPORTANT: keep a queue of audio ahead of the speaker, and do not modify
// a buffer until at least two other buffers have been submitted. M5Unified
// documents three rotating buffers as a safe strategy for generated audio.
// During playback, PLAY-click stops; other buttons enter/exit adjustment mode.
void handlePlayingWheelClick(bool& interrupted){
  if(scrollLong){scrollLong=false;playerFocus=PlayerFocus::Play;playerEditing=false;}
  if(scrollShort){
    scrollShort=false;
    if(playerFocus==PlayerFocus::Play)interrupted=true;
    else playerEditing=!playerEditing;
  }
}
void playClip(){
  if(!captured||!samples)return;
  for(auto* b:playChunks)if(!b)return;
  M5.Mic.end();
  resetPlaybackEffects();
  if(!M5.Speaker.begin()){state=State::Error;errorText="SPEAKER BEGIN FAILED";return;}
  setVolume();state=State::Playing;setStateLed();clearEvents();draw(true);
  double cursor=speed()<0?double(captured-1):0.0;
  tapePosition=captured?float(cursor)/float(captured):0.0f;
  playbackSeconds=0.0f;
  bool interrupted=false,sourceExhausted=false;
  size_t tailSamplesRemaining=0;
  // Four delayed repeats give an audible decay before the speaker stops.
  const size_t maxTailSamples=kSampleRate*3;
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
    handlePlayingWheelClick(interrupted);
    if(interrupted)break;

    // Only submit when the speaker has a free queue slot. The two-slot
    // queue keeps the following segment ready before the first finishes.
    // A third backing buffer prevents overwriting speaker-owned PCM.
    const size_t occupied=M5.Speaker.isPlaying(kChannel);
    const float rate=speed();
    if((!sourceExhausted || tailSamplesRemaining>0) && rate!=0 && occupied<2){
      int16_t* output=playChunks[queued%kPlayBuffers];
      size_t count=0;
      bool loopedInChunk=false;
      size_t samplesSinceBoundary=0;
      for(;count<kPlayChunk;count++){
        if(sourceExhausted){
          if(tailSamplesRemaining==0)break;
          output[count]=applyEcho(0); // drain feedback delay with silence
          --tailSamplesRemaining;
          continue;
        }
        if(cursor<0 || cursor>=double(captured)){
          if(loopMode==LoopMode::Off){
            sourceExhausted=true;
            if(echoEnabled && echoWet>0 && echoHistory)
              tailSamplesRemaining=min(maxTailSamples,size_t(echoDelayMs)*kSampleRate/1000*5);
            if(tailSamplesRemaining){output[count]=applyEcho(0);--tailSamplesRemaining;continue;}
            break;
          }
          if(loopMode==LoopMode::All){
            int next=-1;
            for(int offset=1;offset<=kTrackCount;offset++){
              int candidate=(activeTrack+offset)%kTrackCount;
              if(tracks[candidate].length>0){next=candidate;break;}
            }
            if(next<0){sourceExhausted=true;break;}
            // The queued speaker audio may trail this UI switch by a small
            // buffer; the playback source itself remains continuous.
            if(!selectTrack(next)){sourceExhausted=true;break;}
            updateWaveform();
          }
          cursor=rate<0?double(captured-1):0.0;
          loopedInChunk=true;
          samplesSinceBoundary=0;
        }
        const size_t left=size_t(cursor);
        const size_t right=min(left+1,captured-1);
        const float frac=float(cursor-double(left));
        const float v=samples[left]*(1.0f-frac)+samples[right]*frac;
        output[count]=applyLiveEffects(int16_t(constrain(int32_t(v),-32768,32767)));
        cursor+=double(rate);
        ++samplesSinceBoundary;
      }
      if(count){
        if(!M5.Speaker.playRaw(output,count,kSampleRate,false,1,kChannel,false)){
          state=State::Error;errorText="SPEAKER PLAY FAILED";break;
        }
        ++queued;
        // Reset the transport clock at the exact boundary inside the chunk.
        // Avoid advancing a whole chunk after a loop or track transition.
        if(sourceExhausted)playbackSeconds=float(captured)/float(kSampleRate);
        else if(loopedInChunk)playbackSeconds=float(samplesSinceBoundary)/float(kSampleRate);
        else playbackSeconds+=float(count)/float(kSampleRate);
        tapePosition=captured?constrain(float(cursor/double(captured)),0.0f,1.0f):0.0f;
        // The Goertzel analysis can be expensive; do it on only every
        // fourth chunk, rather than on every 32ms playback chunk.
        if((chunksMade++%4)==0 && count>=kAnalysisWindow){
          analyze(output,count,count/2);
        }
      }
    }
    if(sourceExhausted && tailSamplesRemaining==0 && !M5.Speaker.isPlaying(kChannel))break;
    // Keep button/scroll responsiveness without doing heavy UI work
    // between every generated PCM segment. draw() is frame-limited.
    draw();
    delay(1);
  }
  M5.Speaker.stop();M5.Speaker.end();
  if(state!=State::Error)state=State::Idle;
  setStateLed();clearEvents();draw(true);
}
void updateOrientation(){
  if(state!=State::Idle || page==Page::Charge)return;
  const uint32_t now=millis();
  if(now-lastOrientationPoll<350)return;
  lastOrientationPoll=now;
  uint8_t desired=orientationMode==OrientationMode::Portrait?0:1;
  if(orientationMode==OrientationMode::Auto){
    // M5Unified IMU is optional; hold last orientation if unavailable.
    if(M5.Imu.update()){
      auto data=M5.Imu.getImuData();
      const float x=data.accel.x,y=data.accel.y;
      // StickS3 physical axes: X-dominant is landscape; Y-dominant portrait.
      if(fabsf(x)>fabsf(y)*1.40f && fabsf(x)>0.55f)desired=x>0?1:3;
      else if(fabsf(y)>fabsf(x)*1.40f && fabsf(y)>0.55f)desired=y>0?0:2;
      else desired=displayRotation;
    }else desired=displayRotation;
  }
  if(desired!=orientationCandidate){orientationCandidate=desired;orientationStableAt=now;}
  if(desired==displayRotation||now-orientationStableAt<650)return;
  displayRotation=desired;
  if(!canvasReady)return;
  canvas.deleteSprite();M5.Display.setRotation(displayRotation);
  canvasReady=canvas.createSprite(M5.Display.width(),M5.Display.height())!=nullptr;
  draw(true);
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
    canvas.drawFastHLine(startX,upperY-10,88,themeAccent());
    canvas.drawFastHLine(startX,lowerY+10,88,themeAccent());
    for(int i=0;i<7;i++){
      canvas.fillTriangle(startX+4+i*12,upperY-9,startX+13+i*12,upperY-9,startX+9+i*12,upperY+4,themeAccent());
      canvas.fillTriangle(startX+4+i*12,lowerY+9,startX+13+i*12,lowerY+9,startX+9+i*12,lowerY-4,themeAccent());
    }
    // Small wordmark; crumble its pixels outwards when the jaws meet.
    const uint32_t impactAt=365;
    const bool afterImpact=elapsed>=impactAt;
    // textSize(2) = 12px per character; center using measured width.
    const char* splashTitle="SOUND-CHOMP";
    const int tx=(w-int(strlen(splashTitle))*12)/2,ty=h/2-8;
    if(!afterImpact){
      canvas.setTextSize(2);canvas.setTextColor(themeAccent(),BG);
      canvas.setCursor(tx,ty);canvas.print(splashTitle);
    }else{
      const int spread=min(32,int((elapsed-impactAt)/11));
      for(int i=0;i<60;i++){
        // Deterministic pixel debris, no heap allocations or RNG.
        int dx=((i*37)%23)-11,dy=((i*19)%11)-5;
        int x=w/2+dx*2+((i&1)?spread:-spread);
        int y=h/2+dy+((i&2)?spread/2:-spread/2);
        if(x>=0&&x<w&&y>=0&&y<h)canvas.drawPixel(x,y,(i%4)?WHITE:themeAccent());
      }
    }
    canvas.pushSprite(0,0);
    delay(17);
  }
  clearEvents();
}
} // namespace

void setup(){
  Serial.begin(115200);M5.begin(M5.config());M5.Display.setRotation(1);displayRotation=1;orientationCandidate=1;
  M5.Power.setExtOutput(true);Wire.begin(kGroveSda,kGroveScl);Wire.setClock(100000);
  prefs.begin("soundchomp",false);
  volumePct=uint8_t(constrain(int(prefs.getUChar("volume",65)),0,100));
  brightnessPct=uint8_t(constrain(int(prefs.getUChar("bright",80)),5,100));
  invertedReverse=prefs.getBool("revInvert",false);autoPlay=prefs.getBool("autoPlay",false);
  resetSpeedAfterRecord=prefs.getBool("resetSpd",true);
  orientationMode=static_cast<OrientationMode>(constrain(int(prefs.getUChar("orientation",0)),0,2));
  themeSelection=uint8_t(constrain(int(prefs.getUChar("theme",0)),0,2));
  echoEnabled=prefs.getBool("echoOn",false);
  echoWet=uint8_t(constrain(int(prefs.getUChar("echoWet",35)),0,100));
  echoDelayMs=uint16_t(constrain(int(prefs.getUShort("echoDelay",250)),kEchoMinMs,kEchoMaxMs));
  stutterEnabled=prefs.getBool("stutOn",false);
  stutterSliceMs=uint16_t(constrain(int(prefs.getUShort("stutMs",180)),80,400));
  robotEnabled=prefs.getBool("robotOn",false);
  robotIntensity=uint8_t(constrain(int(prefs.getUChar("robotMix",65)),0,100));
  M5.Display.setBrightness(uint8_t(int(brightnessPct)*255/100));
  for(auto& t:tracks){
    t.pcm=static_cast<int16_t*>(heap_caps_malloc(kMaxSamples*sizeof(int16_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
    t.length=0;
  }
  activeTrack=constrain(int(prefs.getUChar("active",0)),0,kTrackCount-1);
  trackSelection=activeTrack;
  samples=tracks[activeTrack].pcm;
  // The existing 8 MB partition scheme reserves the final ~1.5 MB for a filesystem.
  // WARNING: format-on-failure initializes an unformatted filesystem on first use.
  // It can erase pre-existing incompatible filesystem data in that partition.
  storageReady=LittleFS.begin(true);
  if(storageReady){loadTracks();captured=tracks[activeTrack].length;}
  else Serial.println("LittleFS mount failed: RAM-only operation; no persistent storage");
  for(auto& b:playChunks)b=static_cast<int16_t*>(heap_caps_malloc(kPlayChunk*sizeof(int16_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  echoHistory=static_cast<int16_t*>(heap_caps_calloc(kEchoBufferSamples,sizeof(int16_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  stutterHistory=static_cast<int16_t*>(heap_caps_calloc(kStutterCapacity,sizeof(int16_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  if(!stutterHistory){stutterEnabled=false;Serial.println("Stutter: no PSRAM, effect disabled");}
  if(!echoHistory){echoEnabled=false;Serial.println("Echo: no PSRAM, effect disabled");}
  bool audioBuffersReady=true;for(const auto& t:tracks)audioBuffersReady=audioBuffersReady&&(t.pcm!=nullptr);for(auto* b:playChunks)audioBuffersReady=audioBuffersReady&&(b!=nullptr);
  if(!audioBuffersReady){state=State::Error;errorText="PSRAM ALLOCATION FAILED";Serial.println(errorText);}
  canvas.setColorDepth(16);
  canvasReady=canvas.createSprite(M5.Display.width(),M5.Display.height())!=nullptr;
  if(!canvasReady){M5.Display.fillScreen(TFT_BLACK);M5.Display.drawString("CANVAS FAILED",4,4);}
  scrollConnected=probeScroll();setStateLed();updateWaveform();updateBattery();
  Serial.println("Sound-Chomp Phase 5E - echo tail, stutter and robot effects");splash();draw(true);
}
void loop(){
  service();
  updateOrientation();
  if(effectPreferenceDirty && millis()-effectPreferenceChangedAt>1200){saveEchoSettings();effectPreferenceDirty=false;}
  if(M5.BtnB.wasPressed()&&page==Page::Home){
    page=Page::Chomp;playerFocus=PlayerFocus::Play;playerEditing=false;clearEvents();draw(true);
  }else if(M5.BtnB.wasPressed()&&page!=Page::Home){
    if(page==Page::Tracks&&trackDeleteConfirm){trackDeleteConfirm=false;draw(true);}
    else backHome();
  }
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
    if(state==State::Idle&&scrollShort){
      scrollShort=false;
      if(playerFocus==PlayerFocus::Play)playClip();
      else playerEditing=!playerEditing;
    }
    if(scrollLong){playerFocus=PlayerFocus::Play;playerEditing=false;scrollLong=false;}
  }else if(page==Page::Tracks){
    if(trackDeleteConfirm){
      if(scrollShort||M5.BtnA.wasPressed()){
        tracks[trackSelection].length=0;deleteSavedTrack(trackSelection);
        if(trackSelection==activeTrack){captured=0;updateWaveform();}
        trackDeleteConfirm=false;scrollShort=false;draw(true);
      }
      if(M5.BtnB.wasPressed()){trackDeleteConfirm=false;draw(true);}
    }else{
      if(scrollLong){scrollLong=false;trackDeleteConfirm=true;draw(true);}
      else if(M5.BtnA.wasPressed()||scrollShort){
        scrollShort=false;
        if(selectTrack(trackSelection)){updateWaveform();page=Page::Chomp;draw(true);}
      }
    }
  }else if(page==Page::Settings){
    if(M5.BtnA.wasPressed()||scrollShort){scrollShort=false;settingsEditing=!settingsEditing;if(!settingsEditing)saveSettings();}
    scrollLong=false;
  }else if(page==Page::Charge){
    if(M5.BtnA.wasPressed()||scrollShort){scrollShort=false;chargeLastInteraction=millis();if(chargeScreenOff){chargeScreenOff=false;M5.Display.setBrightness(uint8_t(int(brightnessPct)*255/100));draw(true);}}
    if(!chargeScreenOff&&millis()-chargeLastInteraction>12000){chargeScreenOff=true;M5.Display.setBrightness(0);}
    scrollLong=false;
  }else if(page==Page::Effects){
    if(M5.BtnA.wasPressed()||scrollShort){
      scrollShort=false;
      const uint8_t lastField=effectsSelection==0?2:1;
      if(!effectsEditing){effectsEditing=true;effectsField=0;}
      else if(effectsField<lastField)effectsField++;
      else {effectsEditing=false;saveEchoSettings();}
    }
    scrollLong=false;
  }else if(page==Page::HowTo||page==Page::About){
    scrollShort=false;scrollLong=false;
  }
  draw();delay(5);
}
