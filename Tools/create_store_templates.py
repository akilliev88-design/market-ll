"""Generate only the four Phase A templates. Stats are derived by validate_stores.py."""
import json
from pathlib import Path
from store_specs import STORES

ROOT = Path(__file__).resolve().parents[1]
def generate():
    catalog = json.loads((ROOT/'Config/products.json').read_text(encoding='utf-8-sig'))
    categories = sorted({p['category'] for p in catalog['products'] if p.get('category') and p['category'] not in ('süt','dondurma')})
    stores=[]
    for sid, s in STORES.items():
        w,d=s['footprint']; back=s['back']; rear=d/2-back
        fixtures=[]
        def add(e,c,x,y,yaw=0):
            fixtures.append(dict(id=f'f_{len(fixtures)+1:04d}',equipment=e,category=c,at=[x,y,0],yaw=yaw))
        for col in range(s['cols']):
            for row in range(s['rows']):
                cat=categories[col%len(categories)] if s['cols']>=len(categories) else categories[(col*s['rows']+row)%len(categories)]
                add('gondola_double_1200',cat,(col-(s['cols']-1)/2)*s['xstep'],s['first']+row*s['ystep'],90)
        for n in range(s['walls']):
            add('wall_shelf_2400',categories[n%len(categories)],(-1 if n%2==0 else 1)*(w/2-35),-d/4+(n//2)*250,90 if n%2==0 else -90)
        # Cold walls lie along the rear and both side walls, separate from the dry gondolas.
        for n in range(s['cold']):
            across=max(1,int((w-600)//300))
            if n<across: add('cooler_wall','süt',-w/2+250+n*300,rear-52)
            else: add('cooler_wall','süt',w/2-52,-d/2+480+(n-across)*300,-90)
        for n in range(s['frozen']):
            add('freezer_chest','dondurma',-w/2+140+(n%2)*120,-d/2+520+(n//2)*270,90)
        for n in range(s['checkouts']):
            add('checkout_single','',-w/2+180+n*135,-d/2+155)
        add('tobacco_backbar','',-w/2+180,-d/2+320)
        add('basket_area','',240,-d/2+70)
        if s['format']=='mahalle': add('produce_small','',370,-365)
        elif s['format']=='kucuk': add('produce_large','',w/2-230,rear-170)
        if s['format']!='mahalle': add('pallet_display','',w/2-200,-d/2+360)
        if s['format'] in ('buyuk','hiper'):
            add('deli_counter','',-w/2+220,rear-160)
            add('bakery_shelf','',-w/2+250,rear-450)
            add('cart_area','',w/2-240,-d/2+180)
            for n in range(3): add('produce_large','',w/2-220-n*320,rear-200)
        if s['format']=='hiper':
            for n,e in enumerate(['butcher_counter','fish_counter','home_display','electronics_display','textile_display']):
                add(e,'',-w/2+900+n*330,rear-170)
            add('customer_service','',w/2-700,-d/2+140)
            # The right wall belongs to refrigeration; dry wall shelves move to the left perimeter.
            for n,f in enumerate([f for f in fixtures if f['equipment']=='wall_shelf_2400']):
                f['at']=[-w/2+35,-d/2+550+n*250,0]; f['yaw']=90
        stores.append(dict(id=sid,format=s['format'],name=s['name'],theme=s['theme'],shell=f'/Game/Stores/Shells/SM_Shell_{sid}',
            footprintCm=[w,d],salesAreaM2=w*(d-back)/10000,backroomM2=w*back/10000,ceilingCm=s['ceiling'],
            points=dict(entrance=dict(at=[0,-d/2,0],yaw=90),receiving=dict(at=[w/2-150,d/2,0],yaw=-90),
                playerStart=dict(at=[0,-d/2+(170 if s['format']=='mahalle' else 240 if s['format']=='kucuk' else 340),97],yaw=90),customerSpawn=[[0,-d/2-200,0]],
                backroom=dict(min=[-w/2,rear,0],max=[w/2,d/2,s['ceiling']])),fixtures=fixtures,props=[]))
    (ROOT/'Config/magazalar.json').write_text(json.dumps(dict(schemaVersion=1,stores=stores),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
if __name__=='__main__': generate()
