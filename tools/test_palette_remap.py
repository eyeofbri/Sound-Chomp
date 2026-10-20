#!/usr/bin/env python3
"""Regression test for original green-shade background recoloring."""
from pathlib import Path
from generate_ui_assets import palette_tint, load_png
import json
root=Path(__file__).resolve().parents[1]
assert palette_tint(0,49,0,255,(0,255,0))==(0,49,0)
assert palette_tint(0,49,0,255,(255,0,0))==(49,0,0)
assert palette_tint(0,49,0,255,(0,255,247))[2]>=47
assert palette_tint(60,10,60,255,(0,255,0))==(60,10,60)
assert palette_tint(60,10,60,255,(255,0,0))==(60,10,60)
meta=json.loads((root/'assets/ui/sound_chomp_spritesheet.json').read_text())
for skin,file in [('CLASSIC',root/'assets/ui/sound_chomp_spritesheet.png'),('MINIMAL',root/'assets/skins/minimal.png'),('ARCADE',root/'assets/skins/arcade.png')]:
 w,h,rows=load_png(file)
 assert (w,h)==(meta['meta']['width'],meta['meta']['height'])
 for name,rect in meta['sprites'].items():
  if not name.startswith('bg_'): continue
  x,y,sw,sh=(rect[k] for k in ('x','y','w','h'))
  green=[p for row in rows[y:y+sh] for p in row[x:x+sw] if p[3]>=128 and p[1]>max(p[0],p[2])*1.18 and p[1]>18]
  print(f'{skin}/{name}: {len(green)} green-shaded pixels')
  if skin=='CLASSIC':assert len(green)>500, f'Unexpectedly few green pixels in {name}'
  for p in green[:100]:
   assert palette_tint(*p,(0,255,0))==p[:3]
   assert palette_tint(*p,(255,0,0))[1]==0
print('Green-background palette regression checks passed.')
