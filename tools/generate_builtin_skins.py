#!/usr/bin/env python3
"""Generate built-in alternative skins from fixed-coordinate PNG templates.
Users may recolor pixels but must not move sprites or alter yellow guides.
The locked canonical JSON always supplies the coordinates.
"""
from pathlib import Path
import json
from generate_ui_assets import load_png,rgb565,swapped,cident,palette_tint
ROOT=Path(__file__).resolve().parents[1]
MAP=json.loads((ROOT/'assets/ui/sound_chomp_spritesheet.json').read_text())
OUT=ROOT/'include/generated/builtin_skins.h'
basewidth=MAP['meta']['width'];baseheight=MAP['meta']['height']
lines=['#pragma once','#include <Arduino.h>','#include "generated/ui_assets.h"','namespace ChompBuiltins {']
sets=[]
for style in ('minimal','arcade'):
 w,h,rows=load_png(ROOT/'assets/skins'/f'{style}.png')
 if (w,h)!=(basewidth,baseheight):raise ValueError(f'{style}: must keep original atlas dimensions')
 pointers=[]
 for name,rec in MAP['sprites'].items():
  x,y,sw,sh=[rec[k] for k in ('x','y','w','h')]
  border=[rows[y-1][px] for px in range(x-1,x+sw+1)]+[rows[y+sh][px] for px in range(x-1,x+sw+1)]
  border += [rows[py][x-1] for py in range(y,y+sh)]+[rows[py][x+sw] for py in range(y,y+sh)]
  if any(p!=(255,255,0,255) for p in border):raise ValueError(f'{style}: damaged guide {name}')
  pix=[p for row in rows[y:y+sh] for p in row[x:x+sw]]
  if name.startswith('bg_'):
   for theme,rgb in [('Green',(0,255,0)),('Red',(255,0,0)),('Blue',(0,255,247))]:
    values=[]
    for r,g,b,a in pix:
     r,g,b=palette_tint(r,g,b,a,rgb)
     values.append(swapped(rgb565(r,g,b)))
    ident=f'{style}_{name}_{theme}'
    lines.append(f'const uint16_t {ident}[{sw*sh}] PROGMEM={{')
    for i in range(0,len(values),24):lines.append(' '+','.join(f'0x{v:04X}' for v in values[i:i+24])+',')
    lines.append('};')
  else:
   vals=[0 if a<128 else 0x10000 if (r,g,b)==(255,0,255) else 0x20000 if (r,g,b)==(0,255,255) else 0x30000|rgb565(r,g,b) for r,g,b,a in pix]
   ident=f'{style}_{name}'
   lines.append(f'const uint32_t {ident}[{len(vals)}] PROGMEM={{')
   for i in range(0,len(vals),20):lines.append(' '+','.join(f'0x{v:05X}' for v in vals[i:i+20])+',')
   lines.append('};')
   pointers.append((name,ident))
 sets.append((style,pointers))
lines.append('inline const uint16_t* background(uint8_t skin,uint8_t theme,bool portrait){')
lines.append(' const uint8_t t=theme>2?0:theme;')
for i,(style,_) in enumerate(sets,1):
 lines.append(f' if(skin=={i}){{')
 for j,theme in enumerate(('Green','Red','Blue')):
  lines.append(f'  if(t=={j})return portrait?{style}_bg_portrait_{theme}:{style}_bg_landscape_{theme};')
 lines.append(' }')
lines.extend([' return nullptr;','}'])
lines.append('inline const uint32_t* spritePixels(uint8_t skin,const ChompArt::Sprite& sprite){')
for i,(style,pointers) in enumerate(sets,1):
 lines.append(f' if(skin=={i}){{')
 for name,ident in pointers:lines.append(f'  if(&sprite==&ChompArt::{cident(name)})return {ident};')
 lines.append(' }')
lines.extend([' return sprite.pixels;','}','} // namespace ChompBuiltins'])
OUT.parent.mkdir(parents=True,exist_ok=True)
new=chr(10).join(lines)+chr(10)
if not OUT.exists() or OUT.read_text()!=new:OUT.write_text(new)
print('Generated built-in skins: MINIMAL and ARCADE, fixed sprite coordinates')
