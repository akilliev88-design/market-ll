"""Install authored larger-store layouts using existing product category IDs."""
import json,math
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BASE=ROOT/'AssetInbox/Environment/Stores/Desktop'
def normalize():
    for p in BASE.glob('*/equipment.json'):
        d=json.loads(p.read_text(encoding='utf-8'));plan=d['planogram'];zones=[z for z in d['zones'] if z['face']=='front']
        plan.setdefault('doubleSided',False)
        widths=[z['usableWidthCm'] for z in zones];depths=[z['usableDepthCm'] for z in zones]
        # Standalone manifests described a complete cabinet; the game needs the
        # usable depth of ONE face, not both sides of the cabinet together.
        plan['widthCm']=min(widths) if widths else d['dimensionsMm']['width']/10
        plan['depthCm']=min(depths) if depths else d['dimensionsMm']['depth']/10
        plan.setdefault('frontY',zones[0]['centerCm'][1]-plan['depthCm']/2 if zones else 0)
        plan.setdefault('meshYaw',180)
        plan.setdefault('showCategorySign',False)
        if d['id'] in ('kt_gondola_1250','kt_wall_1250','kt_cosmetic_island_1200','kt_cosmetic_wall_1200'):plan['showCategorySign']=True
        d['family']={'market':'shelf','cosmetic':'shelf','wood':'shelf','electronic':'electronics','glassware':'home'}.get(d['family'],d['family'])
        # Existing household/technology catalogue categories remain unchanged.
        (p).write_text(json.dumps(d,ensure_ascii=False,indent=2),encoding='utf-8')
def arrange():
    normalize()
    file=ROOT/'Config/magazalar.json';doc=json.loads(file.read_text(encoding='utf-8'))
    for s in doc['stores']:
        if s['format']=='mahalle':continue
        w,d=s['footprintCm'];h=s['ceilingCm'];sid=s['id']
        s['name']={'kucuk':'Cam cepheli kompakt market','buyuk':'Taze reyonlu supermarket','hiper':'Bolumlu cam cepheli hipermarket'}[s['format']]
        s['editableShell']=False;s['shell']=f'/Game/Stores/Handmade/Large/{sid}/SM_Handmade_{sid}.SM_Handmade_{sid}'
        s['roof']=f'/Game/Stores/Handmade/Large/{sid}/SM_HandmadeRoof_{sid}.SM_HandmadeRoof_{sid}'
        s['architecture']['outlineCm']=[[-w/2,-d/2],[w/2,-d/2],[w/2,d/2],[-w/2,d/2]]
        s['salesAreaM2']=w*d/10000-s['backroomM2']
        if s['format']=='hiper':
            # Start in the validated clear entrance corridor, outside the
            # nearby front display's footprint.
            entry=s['points']['entrance']['at']
            s['points']['playerStart']['at']=[entry[0],entry[1]+140,97]
        for f in s['fixtures']:
            old=f['equipment']
            if old=='freezer_chest':
                f['equipment']='upright_freezer_2door_1400' if s['format']=='kucuk' else 'upright_freezer_4door_2800'
            elif old=='gondola_double_1200':
                # Leave the narrower 130cm end-to-end runs intact, preventing
                # the 125cm alternative's posts touching adjacent modules.
                f['equipment']='kt_gondola_1250'
            elif old=='bakery_shelf':f['equipment']='bread_selfservice_1200'
            elif old=='produce_large':
                f['equipment']='produce_two_tier';f['category']=''
            elif old=='checkout_single' and s['format']!='kucuk':
                n=int(f['id'].split('_')[-1])
                if n%3==0:f['equipment']='checkout_dark_compact'
            if old=='wall_shelf_2400' and f['category']=='temizlik':f['faceCategories']={'front':'temizlik'}
        # Attach electronics to counters and warehouse tools to the rear stock room.
        props=[p for p in s.get('props',[]) if not p.get('desktopDelivery')]
        def prop(asset,at,yaw=0):
            info=json.loads((BASE/asset/'equipment.json').read_text(encoding='utf-8'))
            props.append(dict(mesh=info['unrealMesh'],at=at,yaw=yaw+180,desktopDelivery=True))
        for f in s['fixtures']:
            if f['equipment']=='checkout_single':prop('pos_dual_display',[f['at'][0],f['at'][1]+65,110])
        rear=s['points']['backroom']['min'][1]
        prop('pallet_jack_orange',[-w/2+210,rear+110,0])
        prop('janitor_cart',[w/2-200,rear+110,0])
        prop('warehouse_rack_2400',[-w/2+180,d/2-65,0])
        s['props']=props
    file.write_text(json.dumps(doc,ensure_ascii=False,indent='\t')+'\n',encoding='utf-8')
    from validate_stores import registry,validate
    cats={p['category'] for p in json.loads((ROOT/'Config/products.json').read_text(encoding='utf-8'))['products']}
    errors=validate(doc,registry(),cats)
    if errors:raise RuntimeError('\n'.join(errors))
    file.write_text(json.dumps(doc,ensure_ascii=False,indent='\t')+'\n',encoding='utf-8')
    print('LARGE_LAYOUTS_VALIDATED=3')
if __name__=='__main__':arrange()
