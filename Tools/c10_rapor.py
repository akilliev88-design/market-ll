"""C10 paired experiment summaries. Money is kuruş; ratios use annual mean list levels.
Usage: python Tools/c10_rapor.py Saved/AutoPlay/C10 Docs/Surec/akislar/C10_veri
Writes machine-readable rows and a review table; incomplete experiments are omitted.
"""
import csv
import hashlib
import json
import shutil
import statistics
import subprocess
import sys
from collections import defaultdict
from datetime import date
from pathlib import Path

STYLES=['Temkinli','Dengeli','Atak']
CASES=['D0','D1','D2','D3','D4','D4b','D5','D6','D6_30']
def read(p):
    with p.open(encoding='utf-8-sig',newline='') as f:return list(csv.DictReader(f))
def group(rows):
    out=defaultdict(list)
    for r in rows:out[r['tarz'],int(r['tohum'])].append(r)
    return out
def day(year):return (date(2011+year,3,7)-date(2011,3,7)).days
def write(p,rows):
    if not rows:return
    with p.open('w',encoding='utf-8-sig',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
def pass_count(rows,fn):return f'{sum(fn(r) for r in rows)}/{len(rows)}'
def avg(rows,key):
    values=[r[key] for r in rows if r[key] is not None]
    return statistics.mean(values) if values else None
def fmt(v,precision=1):return '—' if v is None else f'{v:,.{precision}f}'
def aggregate(folder):
    subprocess.run([sys.executable,str(Path(__file__).with_name('c8_aile_rapor.py')),str(folder)],check=True,stdout=subprocess.DEVNULL)

def main():
    base,out=map(Path,sys.argv[1:3]);out.mkdir(parents=True,exist_ok=True)
    results=[];manifests=[];prefixes={}
    for case in CASES:
        folder=base/case
        if not (folder/'gunluk.csv').exists():continue
        meta=json.loads((folder/'manifest.json').read_text(encoding='utf-8'))
        assert meta['exit_code']==0,(case,meta)
        aggregate(folder)
        days=group(read(folder/'gunluk.csv'));annual=group(read(folder/'aile_yillik.csv'))
        audits=group(read(folder/'c3.csv'));rescues=group(read(folder/'kurtarma.csv'))
        mature=group(read(folder/'olgun_sube.csv'));summary=read(folder/'rapor.md') if False else (folder/'rapor.md').read_text(encoding='utf-8-sig')
        assert 'Tune: ' in summary
        if meta['tune']:
            for kv in meta['tune'].split(','):assert kv.split('=')[0]+'=' in summary
        else:assert 'Tune: C10 defaults' in summary
        assert len(days)==3*meta['seeds']
        for key,ds in days.items():
            assert len(ds)==day(meta['years']),(case,key,len(ds))
            assert [int(r['gun']) for r in ds]==list(range(1,len(ds)+1))
            a=audits[key][0]
            assert int(a['fark_gun'])==int(a['fark_kurus'])==int(a['mutlak_fark_kurus'])==0
            # All stock/transaction audit counts are the final column of the first report table.
            report_row=next(r for r in summary.splitlines() if r.startswith('| '+key[0]+' | '+str(key[1])+' |'))
            assert int(report_row.split('|')[-2].strip())==0
            yearly=annual[key];profit=[int(r['aile_favok_kurus'])-int(r['aile_mudur_gunluk_maliyet_kurus']) for r in yearly]
            real=[v/float(r['liste_duzeyi']) for v,r in zip(profit,yearly)]
            ratio=(real[9]/real[0]) if real[0]>0 else None
            snapshots={int(r['gun']):r for r in ds}
            window=[r for r in mature[key] if day(9)<int(r['gun'])<=day(10)]
            count=sum(int(r['olgun_sube']) for r in window)
            sample_count=sum(int(r['olgun_sube']) for r in mature[key])
            first=next((int(r['gun']) for r in ds if int(r['magaza'])>=2),None)
            r={'deney':case,'tarz':key[0],'tohum':key[1],'yil':meta['years'],'ilk_sube':first,
               'magaza3':int(snapshots[day(3)]['magaza']),'magaza10':int(snapshots[day(10)]['magaza']),
               'sira10':int(snapshots[day(10)]['ulusal_sira']),
               'sira20':int(snapshots[day(20)]['ulusal_sira']) if meta['years']==30 else None,
               'kurtarma':len(rescues[key]),'sikici_donem':int(a['sikici_donem']),
               'reel_10_1':ratio,'reel_son_1':real[-1]/real[0] if real[0]>0 else None,
               'kar1_kurus':profit[0],'kar10_kurus':profit[9],
               'olgun_10_kar30_tl':sum(int(r['kar30_kurus']) for r in window)/count/100 if count else None,
               'olgun_tum_kar30_tl':sum(int(r['kar30_kurus']) for r in mature[key])/sample_count/100 if sample_count else None,
               'olgun_10_sube_ay':count,'son_kasa_kurus':int(ds[-1]['kasa_kurus']),'son_borc_kurus':int(ds[-1]['borc_kurus']),
               'faaliyet_kari_kurus':profit,'reel_faaliyet_kari_kurus':real,'kurtarma_gunleri':[int(p['gun']) for p in rescues[key]]}
            results.append(r)
        if case=='D6':prefixes=days
        if case=='D6_30':
            assert prefixes,'D6 must be analyzed before D6_30'
            for key,ds in days.items():assert ds[:day(10)]==prefixes[key],('combined prefix changed',key)
        dest=out/case;dest.mkdir(exist_ok=True)
        for name in ('manifest.json','aile_yillik.csv','aile_aylik.csv','olgun_sube.csv','kurtarma.csv','kurtarma_yillik.csv','c3.csv','banka.csv','finans_ozet.csv'):
            if (folder/name).exists():shutil.copyfile(folder/name,dest/name)
        meta['evidence']={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in folder.glob('*.csv')}
        manifests.append(meta)
    assert results,'No complete experiments'
    (out/'sonuclar.json').write_text(json.dumps({'manifests':manifests,'rows':results},ensure_ascii=False,indent=2),encoding='utf-8')
    scalar=[{k:v for k,v in r.items() if not isinstance(v,list)} for r in results]
    write(out/'sonuclar.csv',scalar)
    text=['# C10 deney raporu','',
          'Sonuç tablosu: ilk şube/mağaza/sıra üç tohumun ortalaması; parantezde hedefi geçen koşu sayısı. Kurtarma 10 yıllık toplamdır, 30 yıl hedefiyle aynı sayı değildir. Tutarlar TL; yıllık temiz mağaza faaliyet kârı = kendi defter FAVÖK’ü − merkezde ödenen aile müdürü maliyeti. Patron maaşı merkez gideridir.','','## Deney × hedef satırları','']
    for style in STYLES:
        text+=['### '+style,'','| Deney | İlk şube günü | 3. yıl mağaza | 10. yıl mağaza | 10. yıl ulusal | Kurtarma | Reel kâr 10/1 | Olgun şube 30 gün, 10. yıl TL |','|---|---:|---:|---:|---:|---:|---:|---:|']
        for case in CASES:
            rs=[r for r in results if r['deney']==case and r['tarz']==style]
            if not rs:continue
            care=style==STYLES[0];balanced=style==STYLES[1]
            first=fmt(avg(rs,'ilk_sube'))
            first+=' ('+pass_count(rs,lambda r:r['ilk_sube'] is not None and (240 if care else 120)<=r['ilk_sube']<=(426 if care else 244))+')' if care or balanced else ''
            third=fmt(avg(rs,'magaza3'))+(' ('+pass_count(rs,lambda r:r['magaza3']>=10)+')' if balanced else '')
            stores=fmt(avg(rs,'magaza10'))+(' ('+pass_count(rs,lambda r:(15 if care else 60)<=r['magaza10']<=(40 if care else 120))+')' if care or balanced else '')
            rank=fmt(avg(rs,'sira10'))+(' ('+pass_count(rs,lambda r:0<r['sira10']<=(20 if care else 10))+')' if care or balanced else '')
            text.append(f"| {case} | {first} | {third} | {stores} | {rank} | {sum(r['kurtarma'] for r in rs)} | {fmt(avg(rs,'reel_10_1'),3)} ({pass_count(rs,lambda r:r['reel_10_1'] is not None and r['reel_10_1']>=1)}) | {fmt(avg(rs,'olgun_10_kar30_tl'),2)} |")
    text+=['','D4 yalnız dengeli açılış tamponunu; D4b yalnız temkinli tamponunu değiştirir. Diğer tarzlar D0 ile birebir günlük satır eşleşmesiyle kontrol edilir. Atak, kötü oyuncu hedef senaryosu değildir; büyüme/sıra için ✓/✗ verilmez.','','## Koşu başına §5 tablosu','',
           '| Deney / tarz / tohum | İlk şube | 3. yıl mağaza | 10. yıl mağaza | Ulusal 10 / 20 | Kurtarma | Reel kâr 10/1 | Sıkıcı dönem |','|---|---:|---:|---:|---|---:|---:|---:|']
    for r in results:
        care=r['tarz']==STYLES[0];balanced=r['tarz']==STYLES[1]
        def mark(value,ok,applies=True):return str(value)+(' ✓' if ok else ' ✗') if applies else str(value)
        first=mark(r['ilk_sube'],r['ilk_sube'] is not None and (240 if care else 120)<=r['ilk_sube']<=(426 if care else 244),care or balanced)
        third=mark(r['magaza3'],r['magaza3']>=10,balanced)
        stores=mark(r['magaza10'],(15 if care else 60)<=r['magaza10']<=(40 if care else 120),care or balanced)
        rank=mark(r['sira10'],0<r['sira10']<=(20 if care else 10),care or balanced)+' / '+(mark(r['sira20'],0<(r['sira20'] or 0)<=3,balanced) if r['sira20'] else '—')
        rescue=mark(r['kurtarma'],r['kurtarma']<=(5 if not(care or balanced) else 1),r['yil']==30)
        text.append(f"| {r['deney']} / {r['tarz']} / {r['tohum']} | {first} | {third} | {stores} | {rank} | {rescue} | {mark(fmt(r['reel_10_1'],3),r['reel_10_1'] is not None and r['reel_10_1']>=1)} | {mark(r['sikici_donem'],r['sikici_donem']==0)} |")
    text+=['','## Deney ayarları ve doğrulama','']
    for m in manifests:text.append(f"- **{m['experiment']}**: `{m['tune'] or 'C10 defaults'}`, kaynak `{m['source'][:7]}`, {3*m['seeds']} kampanya × {day(m['years'])} gün, {m['seconds']:.1f} sn, çıkış 0.")
    # Unaffected styles form deterministic negative controls for profile-specific knobs.
    for case,affected in [('D4',STYLES[1]),('D4b',STYLES[0])]:
        if (base/case/'gunluk.csv').exists() and (base/'D0/gunluk.csv').exists():
            default=group(read(base/'D0/gunluk.csv'));test=group(read(base/case/'gunluk.csv'))
            for key,rows in test.items():
                if key[0]!=affected:assert rows==default[key],('profile override leaked',case,key)
    text+=['','Para/stok ve işaretli/mutlak defter farkı bütün tamamlanan koşularda 0. D4/D4b’nin etkilenmeyen tarzları D0 ile birebir eşleşir; birleşimin 10 yıllık tohum21 öneki 30 yıl koşusunda birebir doğrulanır. Ham günlük veriler Saved/AutoPlay/C10 içinde, SHA256’ları sonuclar.json içinde. Olgun şube: açık, 90 günü geçmiş; her 30 günde görünen Last30Profit, şube-örnek sayısıyla ağırlıklı. Onuncu yılda olgun şube yoksa — gösterilir; sıfır kâr uydurulmaz. Tüm yıllardaki olgun kâr ayrıca sonuclar.csv’de.','','## İlk dükkânın temiz yıllık faaliyet kârı','']
    for r in results:
        text.append(f"- {r['deney']} / {r['tarz']} / {r['tohum']}: "+' → '.join(f'{v/100:,.2f}' for v in r['faaliyet_kari_kurus'])+' TL.')
    text+=['','## Karar ve sınırlar','','Tek değişkenli sonuçlara göre birleşim seçimi, 30 yıl kurtarma/kârı, derleme/test/smoke ve kaynak satırları final teslim notunda değerlendirilir. Oyun sabitleri değiştirilmez. C9 kampanya formu ve ilk gün yönetim/reklam görüntüleri bu turda değiştirilmedi.']
    report=out.parent/'C10_deney_raporu.md'
    report.write_text('\n'.join(text)+'\n',encoding='utf-8')
    print(json.dumps({'campaigns':len(results),'days':sum(day(r['yil']) for r in results),'experiments':len(manifests),'report':str(report)},ensure_ascii=False))

if __name__=='__main__':main()
