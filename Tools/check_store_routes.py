"""Conservative 20cm floor grid: a 35cm-radius shopper can reach every sales face.

Uses fixture extents, fixed columns/walls, floor polygon and backroom boundary.
This supplements (does not replace) Unreal collision and rendered scene checks.
"""
import json, math
from collections import deque
from pathlib import Path
import numpy as np
from validate_stores import ROOT, registry, inside

def bounds(f,equipment):
    w,d,_=equipment[f['equipment']]['size'];a=math.radians(f['yaw'])
    return (f['at'][0],f['at'][1],abs(math.cos(a))*w/2+abs(math.sin(a))*d/2,abs(math.sin(a))*w/2+abs(math.cos(a))*d/2)

def audit(s,eq):
    step=20; radius=35;w,d=s['footprintCm'];outline=s['architecture']['outlineCm']
    xs=np.arange(-w/2+radius,w/2-radius+1,step);ys=np.arange(-d/2+radius,s['points']['backroom']['min'][1]-radius+1,step)
    X,Y=np.meshgrid(xs,ys);free=np.ones(X.shape,dtype=bool)
    if len(outline)!=4:
        for j,y in enumerate(ys):
            for i,x in enumerate(xs):free[j,i]=all(inside(x+dx,y+dy,outline) for dx,dy in [(radius,0),(-radius,0),(0,radius),(0,-radius)])
    boxes=[bounds(f,eq) for f in s['fixtures']]
    boxes += [(o['at'][0],o['at'][1],o['sizeCm'][0]/2,o['sizeCm'][1]/2) for o in s['architecture'].get('obstacles',[])]
    for x,y,hx,hy in boxes:
        free &= (np.maximum(abs(X-x)-hx,0)**2+np.maximum(abs(Y-y)-hy,0)**2 > radius**2)
    def cell(x,y):return (int(round((y-ys[0])/step)),int(round((x-xs[0])/step)))
    start=cell(*s['points']['playerStart']['at'][:2]);seen=np.zeros(free.shape,dtype=bool)
    errors=[]
    if not free[start]:return ['Player start blocked'],{},free
    q=deque([start]);seen[start]=True
    while q:
        j,i=q.popleft()
        for jj,ii in [(j-1,i),(j+1,i),(j,i-1),(j,i+1)]:
            if 0<=jj<len(ys) and 0<=ii<len(xs) and free[jj,ii] and not seen[jj,ii]:seen[jj,ii]=True;q.append((jj,ii))
    def reachable(x,y):
        j,i=cell(x,y)
        # A grid cell proves connectivity, but rounding must not hide a tight
        # approach at the actual target (e.g. 33cm beside the basket stand).
        exact_clear=all(max(abs(x-bx)-hx,0)**2+max(abs(y-by)-hy,0)**2>radius**2 for bx,by,hx,hy in boxes)
        exact_floor=all(inside(x+dx,y+dy,outline) for dx,dy in [(radius,0),(-radius,0),(0,radius),(0,-radius)]) and y+radius<=s['points']['backroom']['min'][1]
        return exact_clear and exact_floor and 0<=j<len(ys) and 0<=i<len(xs) and seen[j,i]
    checks=0
    for f in s['fixtures']:
        e=eq[f['equipment']];w0,d0,_=e['size'];a=math.radians(f['yaw']);x,y=f['at'][:2]
        info_path=next((ROOT/'AssetInbox/Environment/Stores').rglob(f"{f['equipment']}/equipment.json"),None)
        double=json.loads(info_path.read_text(encoding='utf-8'))['planogram']['doubleSided'] if info_path else False
        sides=[1] if e['family']=='checkout' else [-1,1] if double or e['family']=='produce' else [-1]
        for sign in sides:
            # Sample the whole face away from corner/end posts. Joined banks are
            # one run, so the 5cm module joints are deliberately not aisles.
            points=[]
            for lateral in [-.3*w0,0,.3*w0]:
                local_y=sign*(d0/2+radius+25)
                points.append((x+math.cos(a)*lateral-math.sin(a)*local_y,y+math.sin(a)*lateral+math.cos(a)*local_y))
            checks+=len(points)
            if not all(reachable(px,py) for px,py in points):errors.append(f"Unreachable sales face: {f['id']} {f['equipment']} side={sign}")
    for c in s['layoutDesign']['crossAisles']:
        x0,y0=c['min'];x1,y1=c['max']
        for x,y,hx,hy in boxes:
            if x+hx>x0+.1 and x-hx<x1-.1 and y+hy>y0+.1 and y-hy<y1-.1:errors.append('Cross avenue obstructed')
    for c in s['layoutDesign']['queues']:
        x0,y0=c['min'];x1,y1=c['max']
        for x,y,hx,hy in boxes:
            if x+hx>x0+.1 and x-hx<x1-.1 and y+hy>y0+.1 and y-hy<y1-.1:errors.append('Checkout queue obstructed: '+c['fixture'])
    return errors,dict(store=s['id'],fixtureFacesSampled=checks,reachableFloorCells=int(seen.sum()),radiusCm=radius,gridCm=step),seen

def main():
    doc=json.loads((ROOT/'Config/magazalar.json').read_text(encoding='utf-8'));eq=registry();report=[];errors=[]
    for s in doc['stores']:
        err,r,_=audit(s,eq);report.append(r);errors.extend(s['id']+': '+e for e in err)
    out=ROOT/'Saved/LayoutResearch/routes.json';out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(dict(stores=report,errors=errors),indent=2),encoding='utf-8')
    if errors:raise RuntimeError('\n'.join(errors))
    print(json.dumps(report,indent=2));print('STORE_ROUTES_PASSED=4')
if __name__=='__main__':main()
