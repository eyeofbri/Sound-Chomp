"""Source-level regression guard for splash orientation startup ordering.
Run: python tools/test_splash_orientation_5k3.py
This does not replace M5StickS3 hardware checks.
"""
from pathlib import Path
src=(Path(__file__).resolve().parents[1]/"src/main.cpp").read_text()
startup=src.split("void setup(){",1)[1].split("void loop(){",1)[0]
assert 'displayRotation=chooseStartupRotation();' in startup
assert startup.index('orientationMode=')<startup.index('displayRotation=chooseStartupRotation();')
assert startup.index('displayRotation=chooseStartupRotation();')<startup.index('M5.Display.setRotation(displayRotation);')
assert startup.index('M5.Display.setRotation(displayRotation);')<startup.index('canvas.createSprite(')<startup.index('splash();')
assert 'M5.Display.setRotation(1)' not in startup
assert 'readAutoRotation(displayRotation,desired)' in src
assert 'readAutoRotation(1,candidate)' in src
assert 'high>=3 && high>=runnerUp+2' in src

def orient(x,y,fallback=1):
    if abs(x)>abs(y)*1.40 and abs(x)>0.55:return 1 if x>0 else 3
    if abs(y)>abs(x)*1.40 and abs(y)>0.55:return 0 if y>0 else 2
    return fallback
assert orient(0,1)==0 and orient(0,-1)==2
assert orient(1,0)==1 and orient(-1,0)==3
assert orient(.5,.5)==1
print('Phase 5K.3 splash startup regression checks: PASS (source-level only)')
