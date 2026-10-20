#!/usr/bin/env python3
"""Regenerate Sound-Chomp atlas header automatically during PlatformIO builds.
Pure standard-library PNG reader, supports non-interlaced RGB/RGBA 8-bit PNG.
Yellow guides fall outside mapped crop rectangles and never enter output.
"""
from pathlib import Path
import json, struct, zlib, sys
ROOT=Path(__file__).resolve().parents[1]
ATLAS=ROOT/'assets/ui/sound_chomp_spritesheet.png'
MAP=ROOT/'assets/ui/sound_chomp_spritesheet.json'
OUT=ROOT/'include/generated/ui_assets.h'

def load_png(path):
    data=path.read_bytes()
    if data[:8]!=b'\x89PNG\r\n\x1a\n':raise ValueError('Invalid PNG signature')
    at=8; idat=bytearray(); w=h=bpp=0
    while at<len(data):
        length=struct.unpack_from('>I',data,at)[0]
        typ=data[at+4:at+8]; chunk=data[at+8:at+8+length];at+=length+12
        if typ==b'IHDR':
            w,h,depth,col,compression,filt,interlace=struct.unpack('>IIBBBBB',chunk)
            if depth!=8 or col not in (2,6) or interlace or compression or filt:
                raise ValueError('Atlas must be non-interlaced 8-bit RGB or RGBA PNG')
            bpp=4 if col==6 else 3
        elif typ==b'IDAT':idat.extend(chunk)
        elif typ==b'IEND':break
    raw=zlib.decompress(idat)
    stride=w*bpp; prev=bytearray(stride);offset=0;rows=[]
    for y in range(h):
        kind=raw[offset];offset+=1
        row=bytearray(raw[offset:offset+stride]);offset+=stride
        for i in range(stride):
            a=row[i-bpp] if i>=bpp else 0
            b=prev[i];c=prev[i-bpp] if i>=bpp else 0
            if kind==1:row[i]=(row[i]+a)&255
            elif kind==2:row[i]=(row[i]+b)&255
            elif kind==3:row[i]=(row[i]+((a+b)//2))&255
            elif kind==4:
                p=a+b-c; pa=abs(p-a);pb=abs(p-b);pc=abs(p-c)
                predictor=a if pa<=pb and pa<=pc else b if pb<=pc else c
                row[i]=(row[i]+predictor)&255
            elif kind!=0:raise ValueError('Invalid PNG filter')
        if bpp==4:rows.append([tuple(row[x:x+4]) for x in range(0,stride,4)])
        else:rows.append([(*row[x:x+3],255) for x in range(0,stride,3)])
        prev=row
    return w,h,rows

def palette_tint(r,g,b,a,primary):
    """Legacy green-based background recolor. GREEN uses authored artwork.

    RED/BLUE recolor only visibly green-dominant pixels. This is the
    Phase 5H.3 behavior that passed the user's device tests.
    """
    if primary != (0,255,0) and a >= 128 and g > max(r,b)*1.18 and g > 18:
        strength = g / 255.0
        return tuple(min(255, int(c * strength)) for c in primary)
    return r,g,b

def rgb565(r,g,b):return ((r>>3)<<11)|((g>>2)<<5)|(b>>3)
def swapped(v):return ((v&255)<<8)|(v>>8)
def cident(name):return ''.join(part.capitalize() for part in name.split('_'))

info=json.loads(MAP.read_text())
w,h,rows=load_png(ATLAS)
if (w,h)!=(info['meta']['width'],info['meta']['height']):
    raise ValueError('PNG size does not match manifest')
if info['meta'].get('skin_contract')!='sound-chomp-atlas-v1':
    raise ValueError('Missing/unsupported skin_contract')
if 'icon_teeth' in info['sprites']:
    raise ValueError('Branding must remain outside editable skins: icon_teeth')
used=[]
for name,s in info['sprites'].items():
    if not name.isidentifier() or not all(isinstance(s.get(k),int) for k in ('x','y','w','h')):
        raise ValueError(f'Invalid sprite name/coordinates: {name}')
    x,y,sw,sh=(s[k] for k in ('x','y','w','h'))
    if sw<1 or sh<1 or x<1 or y<1 or x+sw>=w or y+sh>=h:
        raise ValueError(f'Sprite out of bounds: {name}')
    rect=(x-1,y-1,x+sw,y+sh)
    for other,other_rect in used:
        if rect[0]<other_rect[2] and rect[2]>other_rect[0] and rect[1]<other_rect[3] and rect[3]>other_rect[1]:
            raise ValueError(f'Overlapping sprites/guides: {name} and {other}')
    used.append((name,rect))
    # A guide is a 1px yellow frame just outside the crop. It's not firmware art.
    border=[rows[y-1][px] for px in range(x-1,x+sw+1)] + [rows[y+sh][px] for px in range(x-1,x+sw+1)]
    border += [rows[py][x-1] for py in range(y,y+sh)] + [rows[py][x+sw] for py in range(y,y+sh)]
    if any(pixel[:3]!=(255,255,0) or pixel[3]!=255 for pixel in border):
        raise ValueError(f'Missing or damaged yellow guide frame: {name}')
print(f'Validated {len(used)} sprites and guides in {w}x{h} atlas')
if '--validate-only' in sys.argv: sys.exit(0)
lines=['#pragma once','#include <Arduino.h>',
       '// AUTO GENERATED: edit PNG + JSON, not this file.',
       'namespace ChompArt {',
       '// 0=transparent, 0x10000=theme.primary, 0x20000=theme.secondary, 0x30000|RGB565=fixed',
       'struct Sprite { uint16_t w,h; const uint32_t* pixels; };']
for name,s in info['sprites'].items():
    x,y,sw,sh=(s[k] for k in ('x','y','w','h'))
    assert x>=1 and y>=1 and x+sw<w and y+sh<h,(name,s)
    pix=[pixel for row in rows[y:y+sh] for pixel in row[x:x+sw]]
    if name.startswith('bg_'):
        # Background shading uses ONLY the explicit magenta ramp: (N,0,N),
        # N>=20. Fixed-color pixels remain exactly as authored. Each theme
        # receives the same value-scaled accent, including GREEN.
        for suffix,primary in [('Green',(0,255,0)),('Red',(255,0,0)),('Blue',(0,255,247))]:
            values=[]
            for r,g,b,a in pix:
                r,g,b=palette_tint(r,g,b,a,primary)
                values.append(swapped(rgb565(r,g,b)))
            prefix='kRetroLandscape' if name=='bg_landscape' else 'kRetroPortrait'
            ident=prefix+suffix
            lines.append(f'const uint16_t {ident}[{sw*sh}] PROGMEM = {{')
            for i in range(0,len(values),20):lines.append(' '+','.join(f'0x{v:04X}' for v in values[i:i+20])+',')
            lines.append('};')
    else:
        values=[]
        for r,g,b,a in pix:
            value=0 if a<128 else 0x10000 if (r,g,b)==(255,0,255) else 0x20000 if (r,g,b)==(0,255,255) else 0x30000|rgb565(r,g,b)
            values.append(value)
        ident=cident(name)
        lines.append(f'const uint32_t data{ident}[{sw*sh}] PROGMEM = {{')
        for i in range(0,len(values),18):lines.append(' '+','.join(f'0x{v:05X}' for v in values[i:i+18])+',')
        lines.append('};')
        lines.append(f'const Sprite {ident} = {{{sw},{sh},data{ident}}};')
lines.append('} // namespace ChompArt')
OUT.parent.mkdir(parents=True,exist_ok=True)
output='\n'.join(lines)+'\n'
if not OUT.exists() or OUT.read_text()!=output:OUT.write_text(output)
print(f'Generated {OUT.name} from {len(info["sprites"])} atlas regions')
