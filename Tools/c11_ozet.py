"""C11: bot koşusu özeti. Kullanım: python Tools/c11_ozet.py Saved/AutoPlay/C10/C11_E0_s0 ... (her klasörde gunluk.csv, aile_gunluk.csv, kurtarma.csv, c3.csv)."""
import csv,sys,subprocess,statistics
from collections import defaultdict
from datetime import date
from pathlib import Path
def read(p):
    with open(p,encoding='utf-8-sig',newline='') as f: return list(csv.DictReader(f))
def day(y): return (date(2011+y,3,7)-date(2011,3,7)).days
def group(rows):
    g=defaultdict(list)
    for r in rows: g[r['tarz'],int(r['tohum'])].append(r)
    return g
res=[]
for d in sys.argv[1:]:
    d=Path(d)
    subprocess.run([sys.executable,str(Path(__file__).with_name('c8_aile_rapor.py')),str(d)],check=True,stdout=subprocess.DEVNULL)
    days=group(read(d/'gunluk.csv')); ann=group(read(d/'aile_yillik.csv')); resc=group(read(d/'kurtarma.csv')) if (d/'kurtarma.csv').exists() else {}
    c3=group(read(d/'c3.csv'))
    for k,ds in days.items():
        snap={int(r['gun']):r for r in ds}
        Y=len(ds)//365
        yr=ann[k]; prof=[int(r['aile_favok_kurus'])-int(r['aile_mudur_gunluk_maliyet_kurus']) for r in yr]
        real=[p/float(r['liste_duzeyi']) for p,r in zip(prof,yr)]
        first=next((int(r['gun']) for r in ds if int(r['magaza'])>=2),None)
        last=min(day(10),len(ds))
        res.append(dict(k=k,first=first,m3=int(snap[day(3)]['magaza']),m10=int(snap[last]['magaza']),r10=int(snap[last]['ulusal_sira']),
            resc=len(resc.get(k,[])),ratio=(real[min(9,len(real)-1)]/real[0]) if real[0]>0 else None,
            fark=c3[k][0]['fark_kurus'],sik=c3[k][0]['sikici_donem'],
            stores=[int(snap[day(y)]['magaza']) for y in range(1,11) if day(y)<=len(ds)],
            cofavok=[round(int(r['sirket_favok_kurus'])/100/float(r['liste_duzeyi'])) for r in yr],
            shop=[round(x/100) for x in real]))
for r in sorted(res,key=lambda r:(['Temkinli','Dengeli','Atak'].index(r['k'][0]),r['k'][1])):
    print(r['k'],'ilk',r['first'],'m3',r['m3'],'m10',r['m10'],'sira',r['r10'],'kurt',r['resc'],'oran',None if r['ratio'] is None else round(r['ratio'],3),'fark',r['fark'],'sik',r['sik'])
    print('   magaza/yil',r['stores']); print('   sirketFAVOK reel',r['cofavok']); print('   dukkan reel',r['shop'])
