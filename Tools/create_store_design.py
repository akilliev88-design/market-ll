"""Authored department plans: architecture, circulation and fixture groups, G-088 revision 2."""
import json
from pathlib import Path
from store_specs import STORES

ROOT=Path(__file__).resolve().parents[1]
DRY=['içecek','makarna-bakliyat','yağ-salça','atıştırmalık','bisküvi-çikolata','çay-kahve','kağıt','kişisel bakım','temizlik']

def generate():
    stores=[]
    for sid,s in STORES.items():
        w,d=s['footprint']; rear=d/2-s['back']; h=s['ceiling']
        fs=[]; solids=[]; zones=[]; sections=[]
        outline=[[-w/2,-d/2],[w/2,-d/2],[w/2,d/2],[-w/2,d/2]]
        entrance=0
        def add(e,c,x,y,yaw=0):
            fs.append(dict(id=f'f_{len(fs)+1:04d}',equipment=e,category=c,at=[x,y,0],yaw=yaw))
        def gondol(x,y,c,yaw=90): add('gondola_double_1200',c,x,y,yaw)
        def obstacle(oid,kind,x,y,ww,dd): solids.append(dict(id=oid,kind=kind,at=[x,y,0],sizeCm=[ww,dd,h]))
        def zone(name,x,y,ww,dd,finish): zones.append(dict(name=name,at=[x,y],sizeCm=[ww,dd],finish=finish))
        def section(label,x,y,width=300,yaw=-90): sections.append(dict(label=label,at=[x,y,h-75],widthCm=width,yaw=yaw))
        if s['format']=='mahalle':
            outline=[[-700,-280],[-400,-280],[-400,-600],[700,-600],[700,600],[-700,600]]; entrance=200
            for i,y in enumerate([-150,-20,110]): gondol(-530,y,DRY[i])
            for i,x in enumerate([-160,-30,100]): gondol(x,120,DRY[i+3],0)
            for i,x in enumerate([200,330,460]): gondol(x,-110,DRY[i+6],0)
            add('wall_shelf_2400','içecek',-667,170,90)
            for i,y in enumerate([-110,150]): add('wall_shelf_2400',DRY[6+i],667,y,-90)
            add('wall_shelf_2400','atıştırmalık',520,-572,180)
            for x in [-180,140]: add('cooler_wall','süt',x,330)
            add('freezer_chest','dondurma',-475,285)
            add('checkout_single','',-180,-455)
            add('tobacco_backbar','',-330,-310)
            add('produce_small','',470,-365)
            add('basket_area','',330,-500)
            obstacle('column_01','column',10,-210,36,36)
            obstacle('column_02','column',-340,175,38,38)
            obstacle('wall_return','wall',-380,-90,18,150)
            zone('Taze köşe',475,-380,230,180,'wood')
            zone('Giriş ve kasa',160,-440,1020,300,'entry')
            section('MANAV',470,-275,180)
            section('SÜT / KAHVALTI',-50,373,480)
            start=[200,-390,97]
        elif s['format']=='kucuk':
            outline=[[-1100,-950],[800,-950],[800,-650],[1100,-650],[1100,950],[-1100,950]]; entrance=-120
            for col,x in enumerate([-680,-410,-140]):
                for y in [-380,-250,-120]: gondol(x,y,DRY[col])
            for col,x in enumerate([-650,-380,-110,160]):
                for y in [200,330,460]: gondol(x,y,DRY[3+col])
            for i,y in enumerate([-250,0]):
                for x in [430,560,690]: gondol(x,y,DRY[7+i],0)
            gondol(540,460,'kişisel bakım',0)
            for y in [-350,100]: add('wall_shelf_2400','içecek',-1067,y,90)
            for y in [-300,0,300]: add('wall_shelf_2400','temizlik',1067,y,-90)
            for x in [-730,-400,-70]: add('cooler_wall','süt',x,675)
            add('freezer_chest','dondurma',-930,420,90)
            for x in [-760,-490]: add('checkout_single','',x,-760)
            for x,y in [(280,-690),(550,-690),(870,560)]: add('pallet_display','',x,y)
            add('basket_area','',40,-850); add('cart_area','',490,-845,0)
            obstacle('column_01','column',240,160,40,40)
            obstacle('column_02','column',0,-565,40,40)
            obstacle('wall_return','wall',810,-490,18,180)
            zone('Koli teşhir',450,-670,640,330,'bulk')
            zone('Kasa hattı',-615,-740,660,390,'entry')
            zone('Temizlik',600,-80,650,690,'household')
            section('TOPLU ALIŞVERİŞ',440,-520,530)
            section('SOĞUK ÜRÜNLER',-390,720,840)
            section('TEMİZLİK / BAKIM',1060,160,500,180)
            start=[-120,-700,97]
        elif s['format']=='buyuk':
            entrance=800
            for col,x in enumerate([-400,-120,160,440,720,1000,1280,1560]):
                cat=DRY[col%6] if col<6 else DRY[col]
                for y in [-780,-650,-520,-390,80,210,340,470]: gondol(x,y,cat)
            for i,y in enumerate([-1100,-855,-610,-365,-120,125,370]): add('wall_shelf_2400','içecek',-1967,y,90)
            for i,y in enumerate([-1100,-855,-610,-365,-120,125,370,615,860]): add('wall_shelf_2400',DRY[6+i%3],1967,y,-90)
            # Nine connected refrigerators form the cold perimeter; service counters have their own bay.
            for x in [-700,-390,-80,230,540,850,1160,1470,1780]: add('cooler_wall','süt',x,1150)
            for x in [-1800,-1510]:
                for y in [390,720]: add('freezer_chest','dondurma',x,y)
            add('deli_counter','',-1460,1100);add('bakery_shelf','',-1780,1150)
            for x in [-1450,-1080]:
                for y in [-720,-430,-140]: add('produce_large','',x,y)
            for x in [-1250,-980,-710,-340,-70,200]: add('checkout_single','',x,-1290)
            add('tobacco_backbar','',-1690,-1350)
            add('cart_area','',1500,-1330); add('basket_area','',570,-1400)
            add('customer_service','',1150,-1020)
            for x in [460,750]: add('pallet_display','',x,-960)
            zone('Manav avlusu',-1290,-440,1000,1140,'wood')
            zone('Servisli taze ürün',-1470,1050,850,290,'wood')
            zone('Kasa meydanı',-620,-1230,2330,490,'entry')
            zone('Bakım ve temizlik',1570,90,700,1600,'household')
            section('MANAV',-1320,-20,800)
            section('FIRIN / ŞARKÜTERİ',-1490,1190,820)
            section('SÜT / KAHVALTI',560,1190,1650)
            section('KİŞİSEL BAKIM / TEMİZLİK',1950,520,1000,180)
            start=[800,-1180,97]
        else:
            entrance=2750
            lanes=[-1600,-1290,-980,-670,-360,-50,800,1110,1420,1730,2040,2700,3010,3320,3630]
            for col,x in enumerate(lanes):
                cat=DRY[col%6] if col<6 else DRY[3+(col-6)%3] if col<11 else DRY[6+(col-11)%3]
                for y in [-1450,-1320,-1190,-1060,-930,-800,-670,-540,-50,80,210,340,470,600,730,860,1250,1380,1510]: gondol(x,y,cat)
            for x in [-1200,1200,3000]: gondol(x,-1700,'atıştırmalık',0)
            for y in [-1750,-1505,-1260,-1015,-770,-525,-280,-35,210,455]: add('wall_shelf_2400','içecek',-3967,y,90)
            for x in [-3400,-3155,-2910,-2665,-2420,-2175,-1930,-1685,-1440,-1195]: add('wall_shelf_2400','makarna-bakliyat',x,-2467,180)
            for n in range(18): add('cooler_wall','süt',-1500+n*300,1950)
            for n in range(12): add('cooler_wall','süt',3950,-1650+n*300,-90)
            for x in [-3600,-3250,-2900,-2550]:
                for y in [300,650,1000]: add('freezer_chest','dondurma',x,y)
            add('deli_counter','',-3500,1740)
            for x in [-3150,-2800]: add('bakery_shelf','',x,1910)
            add('butcher_counter','',-2380,1870); add('fish_counter','',-2010,1870)
            for x in [-3300,-2850,-2400]:
                for y in [-1500,-1000,-500]: add('produce_large','',x,y)
            for x0 in [-1250,650]:
                for n in range(9): add('checkout_single','',x0+n*170,-2215)
            add('tobacco_backbar','',-1000,-2010)
            add('cart_area','',3500,-2250); add('basket_area','',3050,-2330)
            add('customer_service','',2450,-2360)
            for e,x in [('home_display',2850),('electronics_display',3170),('textile_display',3490)]: add(e,'',x,-1900)
            for x in [-3700,-1900,420,2450]: add('pallet_display','',x,-1840)
            zone('Manav pazarı',-2860,-1010,1380,1540,'wood')
            zone('Fırın ve servis',-2900,1740,1900,500,'wood')
            zone('Ev ve teknoloji',3200,-1860,1180,410,'household')
            zone('Kasa hattı',200,-2200,3420,510,'entry')
            zone('Bakım ve temizlik',3160,-50,1480,2800,'household')
            section('MANAV',-2830,-120,1200)
            section('FIRIN / ŞARKÜTERİ',-3230,1950,1300)
            section('ET / BALIK',-2200,1980,720)
            section('SÜT / KAHVALTI',900,1990,2200)
            section('EV / TEKNOLOJİ / TEKSTİL',3200,-1620,1160)
            section('KİŞİSEL BAKIM / TEMİZLİK',3950,400,1500,180)
            start=[2750,-2140,97]
        area=abs(sum(a[0]*b[1]-b[0]*a[1] for a,b in zip(outline,outline[1:]+outline[:1])))/20000
        back=w*s['back']/10000
        stores.append(dict(id=sid,format=s['format'],name=s['name'],theme=s['theme'],shell=f'/Game/Stores/Shells/SM_Shell_{sid}',roof=f'/Game/Stores/Shells/SM_Roof_{sid}',
            footprintCm=[w,d],salesAreaM2=round(area-back,3),backroomM2=back,ceilingCm=h,
            architecture=dict(outlineCm=outline,obstacles=solids,zones=zones,sections=sections),
            points=dict(entrance=dict(at=[entrance,-d/2,0],yaw=90),receiving=dict(at=[-1050 if sid=='buyuk_01' else -1750 if sid=='hiper_01' else w/2-150,d/2,0],yaw=-90),
                playerStart=dict(at=start,yaw=90),customerSpawn=[[entrance,-d/2-200,0]],backroom=dict(min=[-w/2,rear,0],max=[w/2,d/2,h])),fixtures=fs,props=[]))
    (ROOT/'Config/magazalar.json').write_text(json.dumps(dict(schemaVersion=1,stores=stores),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

if __name__=='__main__':generate()
