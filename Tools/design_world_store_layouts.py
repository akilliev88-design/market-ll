"""Author four distinct retail layouts, preserving imported architecture and catalogue.

REWE: fresh market forecourt and service perimeter; compact convenience stores:
short everyday route; hypermarket: departments and cross avenues. Dimensions are
game design choices, not measurements copied from the reference stores.
"""
import json
from pathlib import Path
from validate_stores import registry, validate

ROOT = Path(__file__).resolve().parents[1]
CATS = ['makarna-bakliyat', 'yağ-salça', 'çay-kahve', 'bisküvi-çikolata',
        'atıştırmalık', 'içecek', 'kağıt', 'temizlik', 'kişisel bakım']

def author(s):
    s['fixtures'] = []
    s['architecture']['sections'] = []
    s['architecture']['zones'] = []
    s['layoutDesign'] = dict(version=1, queues=[], crossAisles=[])
    def add(e, x, y, cat='', yaw=0, back=None):
        f = dict(id='retail_%04d' % (len(s['fixtures'])+1), equipment=e,
                 category=cat, at=[x,y,0], yaw=yaw)
        if back is not None: f['faceCategories'] = dict(front=cat, back=back)
        s['fixtures'].append(f)
        if e in ('checkout_single','checkout_dark_compact'):
            top=y+(125 if e=='checkout_single' else 24.525)
            s['layoutDesign']['queues'].append(dict(fixture=f['id'], min=[x-55,top+10], max=[x+55,top+(130 if s['format']=='mahalle' else 310)]))
        return f
    def bank(x, ys, cat, back, e='kt_gondola_1250'):
        for y in ys: add(e,x,y,cat,90,back)
    def sign(text,x,y,width=250):
        s['architecture']['sections'].append(dict(label=text,at=[x,y,min(s['ceilingCm']-45,320)],widthCm=width,yaw=-90))
    def cross(y0,y1,x0,x1):
        s['layoutDesign']['crossAisles'].append(dict(min=[x0,y0],max=[x1,y1]))
    def zone(name,x,y,w,d,finish):
        s['architecture']['zones'].append(dict(name=name,at=[x,y],sizeCm=[w,d],finish=finish))
    fmt=s['format']
    if fmt=='mahalle':
        s['name']='Gunluk alisveris mahalle marketi'
        for x,a,b in [(-160,CATS[0],CATS[1]),(160,CATS[2],CATS[3]),(430,CATS[4],CATS[5])]:bank(x,[-35,95],a,b)
        for y in [-160,-30,100]:add('kt_wall_1250',-671,y,CATS[5],90)
        for y,c in zip([-140,-10,120],[CATS[8],CATS[7],CATS[6]]):add('kt_wall_1250' if c==CATS[7] else 'kt_cosmetic_wall_1200',671,y,c,-90)
        add('wall_shelf_2400',480,-570,CATS[4],180)
        for x in [-190,100]:add('cooler_wall',x,337,'süt')
        add('upright_freezer_2door_1400',-530,337,'dondurma')
        add('produce_panel_island',480,-360)
        add('checkout_single',-180,-455)
        add('tobacco_backbar',-320,-565,'',180)
        add('basket_area',300,-500,'',180)
        sign('GUNLUK GIDA',180,210,300); sign('SOGUK REYON',-120,330)
        zone('Gunluk taze',480,-360,220,220,'wood')
    elif fmt=='kucuk':
        s['name']='Kompakt sirali market'
        for i,x in enumerate([-770,-410,-50,430,750]):
            a,b=[(CATS[0],CATS[1]),(CATS[2],CATS[3]),(CATS[4],CATS[5]),(CATS[6],CATS[7]),(CATS[8],CATS[8])][i]
            bank(x,[-195,-65,65,195],a,b,'kt_cosmetic_island_1200' if i==4 else 'kt_gondola_1250')
            add('kt_endcap_1000',x,295,a,180)
        for y in [-240,-105,30,165]:add('kt_wood_wall_1200',-1065,y,CATS[5] if y<0 else CATS[2],90)
        # The large detergent carton needs the established wall unit's taller
        # usable shelf clearance; shallow display cabinets stay in personal care.
        for y in [-155,155]:add('wall_shelf_2400',1070,y,CATS[7],-90)
        for x in [-710,-400,-90]:add('cooler_wall',x,680,'süt')
        for x in [290,420,550]:add('bread_selfservice_1200',x,680)
        add('upright_freezer_2door_1400',700,680,'dondurma')
        for x in [-780,-490]:add('checkout_single',x,-795)
        add('produce_long_counter',470,-650);add('basket_area',200,-840,'',180)
        cross(350,610,-990,990)
        sign('GIDA',-410,360);sign('BAKIM VE TEMIZLIK',700,380);sign('SUT VE EKMEK',120,610,350)
        zone('Kompakt kasalar',-635,-790,560,280,'entry');zone('Bakim',750,0,250,620,'household')
    elif fmt=='buyuk':
        s['name']='Taze meydanli supermarket'
        for i,x in enumerate([-850,-500,-150,200,550,900,1250,1600]):
            a,b=[(CATS[0],CATS[1]),(CATS[2],CATS[3]),(CATS[4],CATS[3]),(CATS[5],CATS[5]),(CATS[0],CATS[1]),(CATS[2],CATS[4]),(CATS[6],CATS[7]),(CATS[8],CATS[8])][i]
            e='kt_cosmetic_island_1200' if i==7 else 'kt_gondola_1250'
            bank(x,[-470,-340,-210,-80],a,b,e);bank(x,[385,515,645,775],a,b,e)
            for y in [-565,15,290]:add('kt_endcap_1000',x,y,a,180 if y==15 else 0)
        for x in [-650,-350,-50,250,550,850,1150,1450,1750]:add('cooler_wall',x,1135,'süt')
        for x,e in [(-1690,'produce_slatted_island'),(-1300,'produce_panel_island')]:
            for y in [-760,-390]:add(e,x,y)
        for y in [-1070,-935,-800]:add('kt_wall_1250',-1965,y,CATS[5],90)
        for y in [20,325,630]:add('upright_freezer_4door_2800',-1830,y,'dondurma',90)
        for y in [860,990,1120]:add('bread_selfservice_1200',-1965,y,'',90)
        add('deli_counter',-1640,1130)
        for y in [-650,-515,-380,-245,110,245,380,515,650,785]:add('kt_wall_1250',1965,y,CATS[7],-90)
        for x in [-1300,-1040,-780,-520]:add('checkout_single',x,-1325)
        for x in [-120,130]:add('checkout_dark_compact',x,-1325,'',180)
        add('customer_service',-1680,-1280);add('tobacco_backbar',-1700,-1100)
        add('cart_area',1650,-1330,'',180);add('basket_area',500,-1400,'',180)
        add('drink_cooler_3door_2100',1700,-1050,CATS[5])
        cross(42.5,262.5,-950,1900)
        sign('MANAV',-1500,-200,350);sign('TEMEL GIDA',-300,180,380)
        sign('BAKIM VE EV',1430,180,350);sign('SUT VE SARKUTERI',-60,980,500)
        zone('Manav meydani',-1480,-575,880,850,'wood');zone('Kasa meydani',-620,-1250,2350,450,'entry')
        zone('Bakim ve ev',1650,120,650,1670,'household')
    else:
        s['name']='Bolum caddeli hipermarket'
        starts=[-1510,-365,780]
        for i,x in enumerate([-1750,-1410,-1070,-730,-390,-50,290,630,970,1310,1650,1990]):
            a,b=[(CATS[0],CATS[1]),(CATS[0],CATS[1]),(CATS[2],CATS[3]),(CATS[4],CATS[3]),(CATS[5],CATS[5]),(CATS[2],CATS[4])][i%6]
            for start in starts:bank(x,[start+130*n for n in range(7)],a,b)
            add('kt_endcap_1000',x,-1605,a)
        for x,e,a,b in [(2700,'kt_cosmetic_island_1200',CATS[8],CATS[8]),(3400,'kt_gondola_1250',CATS[7],CATS[6])]:
            for start in starts:bank(x,[start+130*n for n in range(7)],a,b,e)
            add('kt_endcap_1000',x,-1605,a)
        for i,x in enumerate([-3500,-3000,-2500]):
            for j,y in enumerate([-1260,-840,-420,0]):add('produce_panel_island' if i==1 else ('produce_two_tier' if j==3 else 'produce_slatted_island'),x,y)
        for x in [-3570,-3260,-2950,-2640]:
            for y in [500,920,1340]:add('upright_freezer_4door_2800',x,y,'dondurma')
        for e,x in [('deli_counter',-3550),('butcher_counter',-3210),('fish_counter',-2870),('curved_pastry_1800',-2530)]:add(e,x,1930)
        for n in range(19):add('cooler_wall',-1750+n*300,1930,'süt' if n<13 else CATS[5])
        for y in [-1500,-1190,-880,-120,190,960,1270,1580]:add('cooler_wall',3950,y,CATS[5],-90)
        special=[('kt_appliance_bay_1800',-1360),('kt_electronic_table_1600',-1040),('kt_glassware_island_1500',-770),('kt_shoe_island_1000',-180),('textile_display',130),('kt_hardware_1200',380),('kt_glassware_island_1500',980),('kt_electronic_table_1600',1300),('kt_appliance_bay_1800',1560)]
        for e,y in special:add(e,2340,y,'',90)
        for x in [-3500,-3000]:add('curved_pastry_1800',x,-1880)
        for y in [-1510,-1380,-1250,-1120]:add('bread_selfservice_1200',-3965,y,'',90)
        for group,start in enumerate([-1500,-300,950]):
            for n in range(6):add('checkout_dark_compact' if group==2 else 'checkout_single',start+170*n,-2350,'',180 if group==2 else 0)
        add('customer_service',2400,-2350);add('cart_area',3500,-2310,'',180);add('basket_area',3100,-2370,'',180)
        add('tobacco_backbar',-2100,-2360)
        cross(-667.5,-427.5,-2100,3880);cross(477.5,717.5,-2100,3880)
        sign('MANAV VE TAZE',-3000,-1630,650);sign('GIDA',-700,-550,500)
        sign('EV VE TEKNOLOJI',2330,-550,450);sign('BAKIM',2700,600,280);sign('TEMIZLIK',3400,600,300)
        sign('SUT VE SOGUK REYON',-200,1770,600)
        zone('Taze meydan',-3020,-650,1700,1700,'wood');zone('Bolum caddesi',2340,0,560,3350,'household')
        zone('Kasa meydani',100,-2260,4100,400,'entry');zone('Temizlik',3400,0,400,3350,'household')
    # Preserve only warehouse dressing. Checkout terminals follow their new desk.
    s['props']=[p for p in s.get('props',[]) if p['at'][1]>=s['points']['backroom']['min'][1]]
    for f in s['fixtures']:
        if f['equipment']=='checkout_single':
            info=json.loads((ROOT/'AssetInbox/Environment/Stores/Desktop/pos_dual_display/equipment.json').read_text(encoding='utf-8'))
            s['props'].append(dict(mesh=info['unrealMesh'],at=[f['at'][0],f['at'][1]+65,110],yaw=180,desktopDelivery=True))

def main():
    # Delivery metadata used a blanket 25cm clearance even on boards 37-41cm
    # apart. Use actual successive board heights with a conservative 2cm board
    # allowance; keep the top below the measured cabinet envelope.
    for eid in ['kt_gondola_1250','kt_wall_1250','kt_wood_wall_1200','kt_cosmetic_island_1200','kt_cosmetic_wall_1200','kt_endcap_1000']:
        meta=ROOT/'AssetInbox/Environment/Stores/Desktop'/eid/'equipment.json'
        data=json.loads(meta.read_text(encoding='utf-8'));height=data['dimensionsMm']['height']/10
        for face in ['front','back']:
            zones=sorted([z for z in data['zones'] if z['face']==face],key=lambda z:z['centerCm'][2])
            for i,z in enumerate(zones):
                ceiling=zones[i+1]['centerCm'][2]-2 if i+1<len(zones) else height
                z['clearanceHeightCm']=round(max(0,ceiling-z['centerCm'][2]),2)
        meta.write_text(json.dumps(data,ensure_ascii=False,indent=2),encoding='utf-8')
    path=ROOT/'Config/magazalar.json';doc=json.loads(path.read_text(encoding='utf-8'))
    for s in doc['stores']:author(s)
    cats={p['category'] for p in json.loads((ROOT/'Config/products.json').read_text(encoding='utf-8'))['products']}
    errors=validate(doc,registry(),cats)
    if errors:raise RuntimeError('\n'.join(errors))
    path.write_text(json.dumps(doc,ensure_ascii=False,indent='\t')+'\n',encoding='utf-8')
    print('WORLD_LAYOUTS_WRITTEN=4')
if __name__=='__main__':main()
