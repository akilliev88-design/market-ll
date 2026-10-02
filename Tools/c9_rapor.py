"""C9 target curves from genuine commandlet observations; all evidence amounts are kurus."""
import csv
import json
import shutil
import sys
from collections import defaultdict
from datetime import date
from pathlib import Path

sys.path.insert(0,str(Path(__file__).parent))
from c8_aile_rapor import main as aggregate_main

def read(path):
    with path.open(encoding='utf-8-sig',newline='') as f:return list(csv.DictReader(f))
def groups(rows):
    out=defaultdict(list)
    for row in rows:out[(row['tarz'],int(row['tohum']))].append(row)
    return out
def write(path,rows):
    with path.open('w',encoding='utf-8-sig',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
def mark(ok):return '✓' if ok else '✗'
def year_day(year):return (date(2011+year,3,7)-date(2011,3,7)).days
def main():
    ten,thirty,out=map(Path,sys.argv[1:4]);out.mkdir(parents=True,exist_ok=True)
    short_groups=groups(read(ten/'gunluk.csv'));long_groups=groups(read(thirty/'gunluk.csv'))
    for key,rows in long_groups.items():
        assert short_groups[key]==rows[:len(short_groups[key])], ('ten-year prefix differs',key)
    for folder in (ten,thirty):
        sys.argv=['aggregate',str(folder)];aggregate_main()
    curves=[];extras=[]
    for horizon,folder in [(10,ten),(30,thirty)]:
        daily=groups(read(folder/'gunluk.csv'));years=groups(read(folder/'aile_yillik.csv'))
        months=groups(read(folder/'aile_aylik.csv'));events=groups(read(folder/'aile_kararlar.csv'))
        rescues=groups(read(folder/'kurtarma.csv'));audit=groups(read(folder/'c3.csv'))
        for key,days in daily.items():
            style,seed=key
            first=next((int(r['gun']) for r in days if int(r['magaza'])>=2),None)
            snaps={int(r['gun']):r for r in days}
            annual=years[key];profit=[int(r['aile_favok_kurus'])-int(r['aile_mudur_gunluk_maliyet_kurus']) for r in annual]
            core=[int(r['brut_kar_kurus'])-sum(int(r[k]) for k in ('ucret_kurus','sgk_kurus','kira_kurus','isletme_kurus','aile_mudur_gunluk_maliyet_kurus')) for r in annual]
            real_core=[v/float(r['liste_duzeyi']) for v,r in zip(core,annual)]
            plans=len(rescues[key]);c=audit[key][0]
            assert int(c['fark_gun'])==int(c['fark_kurus'])==int(c['mutlak_fark_kurus'])==0
            row={'tarz':style,'tohum':seed,'yil':horizon,'ilk_sube_gunu':first,'magaza_3':int(snaps[year_day(3)]['magaza']),
                 'magaza_10':int(snaps[year_day(10)]['magaza']),'sira_10':int(snaps[year_day(10)]['ulusal_sira']),
                 'sira_20':int(snaps[year_day(20)]['ulusal_sira']) if horizon==30 else None,
                 'kurtarma':plans,'sikici_donem':int(c['sikici_donem']),'en_uzun_sessizlik':int(c['en_uzun_sessizlik']),
                 'ilk_dukkan_faaliyet_kari_kurus':profit,'cekirdek_ust_sinir_kurus':core,
                 'reel_cekirdek_ust_sinir_kurus':real_core,'kar_egrisi_buyuyor':real_core[-1]>=real_core[0] and core[-1]>0}
            curves.append(row)
            extra={'tarz':style,'tohum':seed,'yil':horizon,'acil_mal':sum(r['tur']=='lifeline' for r in events[key]),
                   'mal_parasi_uyari':sum(r['tur']=='goods_warning' for r in events[key]),
                   'bos_raf_ayi':sum(int(r['bos_raf_istek'])>0 for r in months[key]),
                   'bos_raf_istek':sum(int(r['bos_raf_istek']) for r in months[key]),
                   'patron_net_maas_kurus':sum(int(r['patron_net_maas_kurus']) for r in annual),
                   'patron_net_kar_payi_kurus':sum(int(r['patron_net_kar_payi_kurus']) for r in annual),
                   'son_servet_kurus':int(annual[-1]['servet_kurus'])}
            extras.append(extra)
        for file in ['aile_yillik.csv','aile_aylik.csv','aile_kararlar.csv','c3.csv','kurtarma.csv','kurtarma_yillik.csv','banka.csv','finans_ozet.csv','rapor.md']:
            p=folder/file
            if p.exists():shutil.copyfile(p,out/f'{horizon}_{file}')
    (out/'hedefler.json').write_text(json.dumps({'kaynaklar':[str(ten.resolve()),str(thirty.resolve())],'egriler':curves,'ek_olcum':extras},ensure_ascii=False,indent=2),encoding='utf-8')
    write(out/'ek_olcum.csv',extras)
    text=['# C9 çekirdek denge raporu','', '## §5 hedef eğrileri','',
          '✓/✗ hedef karşılaştırmasıdır; test sonucu değildir. Atak bir kötü oyun senaryosu değildir; sayısal ilk şube/mağaza/sıra hedefi atanmadı. 10 yıllık koşular 21/22/23, 30 yıllık koşular 21. Tutarlar nominal TL.', '',
          '| Tarz / tohum / süre | İlk şube günü | 3. yıl mağaza | 10. yıl mağaza | 10. yıl ulusal | 20. yıl ulusal | Kurtarma | Kâr eğrisi | Sıkıcı dönem |',
          '|---|---:|---:|---:|---:|---:|---:|---|---:|']
    for r in curves:
        style=r['tarz'];care=style=='Temkinli';balanced=style=='Dengeli';first=r['ilk_sube_gunu']
        firstok=first is not None and (240<=first<=426 if care else 120<=first<=244)
        firstcell=f'{first or "açılmadı"} {mark(firstok)}' if care or balanced else str(first or 'açılmadı')
        stores=f"{r['magaza_10']} {mark(15<=r['magaza_10']<=40 if care else 60<=r['magaza_10']<=120)}" if care or balanced else str(r['magaza_10'])
        third=f"{r['magaza_3']} {mark(r['magaza_3']>=10)}" if balanced else str(r['magaza_3'])
        rank=f"{r['sira_10']} {mark(0<r['sira_10']<=(20 if care else 10))}" if care or balanced else str(r['sira_10'])
        rank20=f"{r['sira_20']} {mark(0<r['sira_20']<=3)}" if balanced and r['sira_20'] else str(r['sira_20'] or '—')
        rescue=f"{r['kurtarma']} {mark(r['kurtarma']<=(5 if style=='Atak' else 1))}" if r['yil']==30 else f"{r['kurtarma']} (10 yıl)"
        text.append(f"| {style} / {r['tohum']} / {r['yil']} yıl | {firstcell} | {third} | {stores} | {rank} | {rank20} | {rescue} | {mark(r['kar_egrisi_buyuyor'])}* | {r['sikici_donem']} {mark(r['sikici_donem']==0)} |")
    text+=['','İlk şube takvim hedeflerinin gün karşılığı yaklaşık 4–8 ay = 120–244, 8–14 ay = 240–426. Üçüncü yıl 10 mağaza alt sınır kabul edildi. Şube sayısı açık şubeler + ilk mağaza; kapanmış kayıtlar sayılmaz.',
           '', '* Kâr kıyası, aşağıdaki mağaza çekirdeği üst sınırının son yılda enflasyondan arındırılmış ilk yıl düzeyini koruyup korumadığıdır. Yılın ortalama katalog liste düzeyi kullanıldı; bütün yılların monoton artması şart koşulmadı. Ağ giderleri ilk dükkân defterine karıştığı için gerçek ilk mağaza neti ayrıca doğrulanamıyor. ✓ varsa dahi gider dağıtımı düzeltilmeden hedef tamamlanmış sayılmaz.', '', '## İlk dükkânın yıllık faaliyet kârı', '',
           'Birinci dizi aile dükkânı defter FAVÖK’ü − aile müdürünün toplam işveren maliyeti. İkinci dizi brüt kâr − personel/SGK/kira/işletme/aile müdürü; ayrıştırılamayan diğer giderleri içermediği için üst sınırdır. Brüt kâr zaten fire ve stok kaybını içerir. Patron gideri merkezde; patron sonrası ayrı sütun ham yıllık CSV’de. Sütun sırası oyun yılıdır.', '']
    for r in curves:
        text.append(f"- {r['tarz']} / {r['tohum']} / {r['yil']} yıl, defter: "+' → '.join(f'{v/100:,.2f}' for v in r['ilk_dukkan_faaliyet_kari_kurus'])+' TL.')
        text.append('  Mağaza çekirdeği üst sınırı: '+' → '.join(f'{v/100:,.2f}' for v in r['cekirdek_ust_sinir_kurus'])+' TL.')
    text+=['','## Mal ve patron','', '| Tarz / tohum / süre | Acil mal | Boş raf ayı | Mal parası uyarısı | Net maaş TL | Net kâr payı TL | Son servet TL |', '|---|---:|---:|---:|---:|---:|---:|']
    for r in extras:text.append(f"| {r['tarz']} / {r['tohum']} / {r['yil']} | {r['acil_mal']} | {r['bos_raf_ayi']} | {r['mal_parasi_uyari']} | {r['patron_net_maas_kurus']/100:,.2f} | {r['patron_net_kar_payi_kurus']/100:,.2f} | {r['son_servet_kurus']/100:,.2f} |")
    text+=['','Boş raf ayı, en az bir karşılanamayan ürün isteği bulunan takvim ayıdır; bütün ay rafların boş olduğu anlamına gelmez. İlk/son takvim ayı kısmidir. Toptancı acil malı ve uyarı ilgili gün alanının değişmesinden sayılır, haber metninden tahmin edilmez. Aynı tohumun 10 yıllık koşusu 30 yılın tekrarlanan önekidir; toplamları bağımsız olay gibi toplamayın. Servet yaşam giderlerinden sonraki bakiye, maaşların toplamı değildir.', '', '## Doğrulama ve öneriler', '', 'DERLE/TEST/Smoke, kaynak satırları ve menü görsel incelemesi teslim notunda tamamlanır. Oyun sabitleri değişmedi.']
    (out.parent/'C9_cekirdek_denge_rapor.md').write_text('\n'.join(text)+'\n',encoding='utf-8')
    print(json.dumps(curves,ensure_ascii=False))
if __name__=='__main__':main()
