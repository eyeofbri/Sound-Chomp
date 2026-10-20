#!/usr/bin/env python3
"""Opt-in, deterministic compact atlas packing. Requires Pillow (only for repacking).
Rewrites PNG + JSON together, keeping 1px yellow guides OUTSIDE all crop rectangles.
Never run as part of a firmware build: artists control when coordinates change.
"""
from pathlib import Path
import argparse, json, sys
from PIL import Image, ImageDraw
ROOT=Path(__file__).resolve().parents[1]
PNG=ROOT/'assets/ui/sound_chomp_spritesheet.png'
MANIFEST=ROOT/'assets/ui/sound_chomp_spritesheet.json'

def place_rects(items, width):
    # Deterministic MaxRects-like free-space packing.
    height=max(width*4,1024)
    free=[(0,0,width,height)]
    out={}
    for name,w,h in items:
        candidates=[(short,long,y,x,i) for i,(x,y,fw,fh) in enumerate(free)
                    if fw>=w and fh>=h
                    for short,long in [(min(fw-w,fh-h),max(fw-w,fh-h))]]
        if not candidates:return None
        _,_,y,x,_=min(candidates)
        out[name]=(x,y,w,h)
        used=(x,y,w,h)
        next_free=[]
        for ax,ay,aw,ah in free:
            bx,by,bw,bh=used
            if bx>=ax+aw or bx+bw<=ax or by>=ay+ah or by+bh<=ay:
                next_free.append((ax,ay,aw,ah));continue
            if bx>ax: next_free.append((ax,ay,bx-ax,ah))
            if bx+bw<ax+aw:next_free.append((bx+bw,ay,ax+aw-bx-bw,ah))
            if by>ay:next_free.append((ax,ay,aw,by-ay))
            if by+bh<ay+ah:next_free.append((ax,by+bh,aw,ay+ah-by-bh))
        free=[r for i,r in enumerate(next_free) if not any(
            i!=j and other[0]<=r[0] and other[1]<=r[1] and other[0]+other[2]>=r[0]+r[2] and other[1]+other[3]>=r[1]+r[3]
            for j,other in enumerate(next_free))]
    return out

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dry-run',action='store_true',help='show output dimensions without changing files')
    args=parser.parse_args()
    data=json.loads(MANIFEST.read_text()); source=Image.open(PNG).convert('RGBA')
    slices={k:source.crop((v['x'],v['y'],v['x']+v['w'],v['y']+v['h'])) for k,v in data['sprites'].items()}
    # Margin 1px guide and 1px gap on each side => w+4,h+4.
    ordered=sorted(((k,im.width+4,im.height+4) for k,im in slices.items()),key=lambda a:(-a[2],-a[1],a[0]))
    min_width=max(w for _,w,_ in ordered)
    candidates=[]
    for width in range(min_width,513):
        layout=place_rects(ordered,width)
        if layout:
            used_width=max(x+w for x,y,w,h in layout.values())
            used_height=max(y+h for x,y,w,h in layout.values())
            candidates.append((used_width*used_height,used_height,used_width,layout))
    if not candidates:raise SystemExit('No placement found')
    _,height,width,layout=min(candidates,key=lambda a:(a[0],a[1],a[2]))
    print(f'Atlas: {source.width}x{source.height} -> {width}x{height}; sprites={len(slices)}')
    if args.dry_run:return
    out=Image.new('RGBA',(width,height),(0,0,0,0)); d=ImageDraw.Draw(out)
    for name in sorted(slices):
        x,y,tw,th=layout[name]; im=slices[name]
        sx,sy=x+2,y+2
        d.rectangle((sx-1,sy-1,sx+im.width,sy+im.height),outline=(255,255,0,255))
        out.paste(im,(sx,sy))
        data['sprites'][name]['x']=sx;data['sprites'][name]['y']=sy
    data['meta']['width']=width;data['meta']['height']=height
    data['meta']['packing']='deterministic MaxRects, 1px border, 1px padding; optional tools/pack_spritesheet.py'
    out.save(PNG,optimize=True)
    MANIFEST.write_text(json.dumps(data,indent=2)+'\n')
    print('Updated PNG and JSON in place; build now regenerates firmware assets')

if __name__=='__main__':main()
