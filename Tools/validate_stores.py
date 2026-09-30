"""Validate G-088 templates and calculate stats from physical equipment, never hand-written totals."""
import argparse, json, math, re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BANDS={'mahalle':(80,200,1,2,25,60,4,10),'kucuk':(250,400,2,3,60,110,8,16),'buyuk':(600,1500,4,8,180,400,25,60),'hiper':(3000,6000,15,30,700,1500,100,200)}
REQUIRED={'mahalle':{'produce','tobacco'},'kucuk':set(),'buyuk':{'produce','tobacco','deli','bakery','freezer'},'hiper':{'produce','tobacco','deli','bakery','freezer','butcher','fish','home','electronics','textile','service'}}

def inside(x,y,outline):
    hit=False
    for a,b in zip(outline,outline[1:]+outline[:1]):
        cross=(x-a[0])*(b[1]-a[1])-(y-a[1])*(b[0]-a[0])
        if abs(cross)<.01 and min(a[0],b[0])-.01<=x<=max(a[0],b[0])+.01 and min(a[1],b[1])-.01<=y<=max(a[1],b[1])+.01:return True
        if (a[1]>y)!=(b[1]>y) and x<(b[0]-a[0])*(y-a[1])/(b[1]-a[1])+a[0]:hit=not hit
    return hit

def floor_area(outline):return abs(sum(a[0]*b[1]-b[0]*a[1] for a,b in zip(outline,outline[1:]+outline[:1])))/20000
def registry(root=ROOT):
    # Established modules retain their existing physical behaviour.
    result={'gondola_double_1200':dict(family='shelf',size=[120,90,160],front=2.4),
            'wall_shelf_2400':dict(family='shelf',size=[240,51.8,222],front=2.4)}
    for p in sorted((root/'AssetInbox/Environment/Stores').rglob('equipment.json')):
        obj=json.loads(p.read_text(encoding='utf-8'))
        if 'planogram' not in obj: continue
        if obj['id'] in result: raise ValueError(f'Duplicate equipment: {obj["id"]}')
        size=[obj['dimensionsMm'][k]/10 for k in ('width','depth','height')]
        result[obj['id']]=dict(family=obj['family'],size=size,front=size[0]/100*(2 if obj['planogram']['doubleSided'] else 1),checkouts=obj.get('checkouts',0))
    return result
def validate(document,equipment,categories):
    errors=[]; ids=set()
    if document.get('schemaVersion')!=1: errors.append('Unsupported schemaVersion')
    for s in document.get('stores',[]):
        sid=s.get('id',''); fmt=s.get('format',''); prefix=sid+': '
        def fail(message): errors.append(prefix+message)
        if not re.fullmatch(re.escape(fmt)+r'_\d{2}',sid) or sid in ids: fail('Invalid or duplicate id')
        ids.add(sid)
        if fmt not in BANDS: fail('Unknown format'); continue
        w,d=s['footprintCm']; area=s['salesAreaM2']; back=s['backroomM2']
        if not all(math.isfinite(v) and v>0 for v in (w,d,area,back,s['ceilingCm'])): fail('Invalid dimensions')
        architecture=s.get('architecture',{})
        outline=architecture.get('outlineCm',[[-w/2,-d/2],[w/2,-d/2],[w/2,d/2],[-w/2,d/2]])
        if len(outline)<3 or any(len(v)!=2 or any(not math.isfinite(n) for n in v) for v in outline): fail('Invalid floor outline');continue
        if abs(area+back-floor_area(outline))>.02: fail('Sales + backroom area must match actual floor polygon')
        p=s['points']; lo=p['backroom']['min']; hi=p['backroom']['max']
        if any(lo[i]>=hi[i] for i in range(3)): fail('Invalid backroom bounds')
        if abs((hi[0]-lo[0])*(hi[1]-lo[1])/10000-back)>.02: fail('Backroom bounds disagree with area')
        for key in ('entrance','receiving','playerStart'):
            at=p[key]['at']
            if len(at)!=3 or any(not math.isfinite(v) for v in at) or not inside(at[0],at[1],outline): fail('Invalid point '+key)
        if not p['customerSpawn']: fail('No customer spawn')
        stats=dict(shelfFrontM=0.,coolerM=0.,freezerM=0.,produceM2=0.,counters=[],checkouts=0,selfCheckouts=0,backroomPallets=int(back/3.5))
        fixture_ids=set(); families=set(); aisle=set(); boxes=[]
        for o in architecture.get('obstacles',[]):
            oid=o['id'];x,y,z=o['at'];ew,ed,eh=o['sizeCm']
            if oid in fixture_ids or o['kind'] not in ('column','wall') or min(ew,ed,eh)<=0 or eh>s['ceilingCm']: fail('Invalid structural obstacle '+oid)
            fixture_ids.add(oid); boxes.append((oid,x,y,ew/2,ed/2))
            if any(not inside(x+dx*ew/2,y+dy*ed/2,outline) for dx in (-1,1) for dy in (-1,1)): fail('Structural obstacle outside shell '+oid)
        for f in s['fixtures']:
            fid=f['id']; cat=f['category']; eid=f['equipment']
            if not re.fullmatch('[a-z][a-z0-9_]{2,39}',fid) or fid in fixture_ids: fail('Invalid/duplicate fixture '+fid)
            fixture_ids.add(fid)
            if eid not in equipment: fail('Unknown equipment '+eid); continue
            e=equipment[eid]; fam=e['family']; families.add(fam)
            if cat and cat not in categories: fail('Unknown category '+cat)
            for c in f.get('faceCategories',{}).values():
                if c and c not in categories: fail('Unknown face category '+c)
            aisle.add(cat)
            x,y,z=f['at']; angle=math.radians(f['yaw']); ew,ed,_=e['size']
            bx=abs(math.cos(angle))*ew/2+abs(math.sin(angle))*ed/2
            by=abs(math.sin(angle))*ew/2+abs(math.cos(angle))*ed/2
            if any(not inside(x+dx*bx,y+dy*by,outline) for dx in (-1,0,1) for dy in (-1,0,1)) or z!=0: fail('Fixture outside shell '+fid)
            boxes.append((fid,x,y,bx,by))
            if x+bx>lo[0]+.5 and x-bx<hi[0]-.5 and y+by>lo[1]+.5 and y-by<hi[1]-.5: fail('Sales fixture in backroom '+fid)
            if fam in ('shelf','bakery','tobacco'): stats['shelfFrontM']+=e['front']
            if fam=='cooler': stats['coolerM']+=ew/100
            if fam=='freezer': stats['freezerM']+=ew/100
            if fam=='produce': stats['produceM2']+=ew*ed/10000
            if fam in ('deli','butcher','fish','service'): stats['counters'].append(fam)
            stats['checkouts']+=e.get('checkouts',0)
        for k,v in stats.items():
            if isinstance(v,float): stats[k]=round(v,3)
        stats['counters']=sorted(set(stats['counters']))
        s['stats']=stats
        for i,a in enumerate(boxes):
            px,py,_=p['playerStart']['at']
            if math.hypot(max(0,abs(px-a[1])-a[3]),max(0,abs(py-a[2])-a[4]))<44: fail('Player starts inside fixture '+a[0])
            for b in boxes[i+1:]:
                if abs(a[1]-b[1]) < a[3]+b[3]-.5 and abs(a[2]-b[2]) < a[4]+b[4]-.5: fail('Overlapping fixtures '+a[0]+' / '+b[0])
        band=BANDS[fmt]
        for value,minimum,maximum,label in [(area,*band[:2],'salesAreaM2'),(stats['checkouts'],*band[2:4],'checkouts'),(stats['shelfFrontM'],*band[4:6],'shelfFrontM'),(stats['coolerM']+stats['freezerM'],*band[6:8],'cold front')]:
            if not minimum<=value<=maximum: fail(f'{label} {value} outside {minimum}–{maximum}')
        # "Dry grocery" is a department, represented by the catalog's pasta/pulses category.
        for cat in ('makarna-bakliyat','içecek','süt'):
            if cat not in aisle: fail('Required aisle missing: '+cat)
        if fmt!='mahalle' and 'temizlik' not in aisle: fail('Cleaning aisle missing')
        if fmt in ('buyuk','hiper') and 'kişisel bakım' not in aisle: fail('Personal care missing')
        if 'cooler' not in families: fail('Dairy cooler missing')
        for fam in REQUIRED[fmt]-families: fail('Required department missing: '+fam)
    return errors
def main():
    parser=argparse.ArgumentParser(); parser.add_argument('--write',action='store_true'); parser.add_argument('path',nargs='?',default=ROOT/'Config/magazalar.json'); args=parser.parse_args()
    path=Path(args.path); doc=json.loads(path.read_text(encoding='utf-8'))
    cats={p['category'] for p in json.loads((ROOT/'Config/products.json').read_text(encoding='utf-8-sig'))['products']}
    old_stats={s['id']:s.get('stats') for s in doc['stores']}
    errors=validate(doc,registry(),cats)
    if not args.write:
        errors.extend(s['id']+': stats stale; run --write' for s in doc['stores'] if old_stats[s['id']]!=s['stats'])
    if errors: print('\n'.join(errors)); raise SystemExit(1)
    if args.write: path.write_text(json.dumps(doc,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    for s in doc['stores']: print(s['id'],s['salesAreaM2'],s['stats'])
    print('STORES_VALIDATED='+str(len(doc['stores'])))
if __name__=='__main__': main()
