#!/usr/bin/env python3
"""Generate all RGB565 backgrounds and tintable UI sprites from canonical PNG.

Run from any cwd: python tools/generate_ui_assets.py
Source crop rectangles NEVER include magenta guide borders.
Reserved opaque source colors: #FF00FF primary accent, #00FFFF secondary accent.
"""
from pathlib import Path
from PIL import Image
import json
ROOT=Path(__file__).resolve().parents[1]
ATLAS=ROOT/'assets/ui/sound_chomp_spritesheet.png'
MAP=ROOT/'assets/ui/sound_chomp_spritesheet.json'
OUT=ROOT/'include/generated/ui_assets.h'
info=json.loads(MAP.read_text())
im=Image.open(ATLAS).convert('RGBA')
assert im.size==(info['meta']['width'],info['meta']['height'])
sprites=info['sprites']
def normal565(r,g,b): return ((r>>3)<<11)|((g>>2)<<5)|(b>>3)
def swapped(v):return ((v&255)<<8)|(v>>8)
def cident(name): return ''.join(part.capitalize() for part in name.split('_'))
lines=['#pragma once','#include <Arduino.h>','// AUTO-GENERATED; edit assets/ui/sound_chomp_spritesheet.png, never this header.','namespace ChompArt {',
'// Pixel encoding: 0=transparent, 0x10000=primary, 0x20000=secondary,',
'// 0x30000 | RGB565=opaque fixed color (including black).',
'struct Sprite { uint16_t w,h; const uint32_t* pixels; };']
for name,s in sprites.items():
    x,y,w,h=(s[k] for k in ('x','y','w','h'))
    assert x>=1 and y>=1 and x+w<im.width and y+h<im.height,(name,s)
    crop=im.crop((x,y,x+w,y+h))
    pix=list(crop.getdata())
    if name.startswith('bg_'):
        # ESP32 M5Canvas pushImage() consumes byte-swapped RGB565 with this config.
        v=[swapped(normal565(r,g,b)) for r,g,b,a in pix]
        identifier='kRetroLandscape' if name=='bg_landscape' else 'kRetroPortrait'
        lines.append(f'const uint16_t {identifier}[{w*h}] PROGMEM = {{')
        for i in range(0,len(v),16):lines.append('  '+','.join(f'0x{n:04X}' for n in v[i:i+16])+',')
        lines.append('};')
    else:
        values=[]
        for r,g,b,a in pix:
            if a<128: value=0
            elif (r,g,b)==(255,0,255):value=0x10000
            elif (r,g,b)==(0,255,255):value=0x20000
            else:value=0x30000|normal565(r,g,b)
            values.append(value)
        ident=cident(name)
        lines.append(f'const uint32_t data{ident}[{w*h}] PROGMEM = {{')
        for i in range(0,len(values),14):lines.append('  '+','.join(f'0x{n:05X}' for n in values[i:i+14])+',')
        lines.append('};')
        lines.append(f'const Sprite {ident} = {{{w},{h},data{ident}}};')
lines.append('} // namespace ChompArt')
OUT.parent.mkdir(parents=True,exist_ok=True);OUT.write_text('\n'.join(lines)+'\n')
print(f'Generated {OUT} from {len(sprites)} sprites')
