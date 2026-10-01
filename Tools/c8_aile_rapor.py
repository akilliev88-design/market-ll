"""Aggregate the bot's daily observations into calendar months and game years.
All money stays in kurus; fixed-price basket means are daily equal-weight means.
Snapshots and payable/cash balances are end-of-period values, never summed.
"""
import argparse
import csv
from collections import defaultdict
from pathlib import Path

SUM = set('musteri alan kaybolan satilan_adet pahali_istek bos_raf_istek rafta_yok_istek bekleme_kaybi kart_kaybi ciro_kurus mal_maliyeti_kurus brut_kar_kurus ucret_kurus sgk_kurus kira_kurus isletme_kurus fire_kurus aile_diger_gider_kurus aile_favok_kurus sirket_favok_kurus sirket_net_kurus faiz_kurus vergi_kurus vergi_odeme_kurus patron_net_maas_kurus patron_net_kar_payi_kurus siparis_kurus patron_sirket_maliyeti_kurus'.split())
MEAN = set('calisan kasiyer gorevli ik musavir raf_ortalama_kurus liste_ortalama_kurus alis_ortalama_kurus stok_maliyeti_ortalama_kurus bot_hedef_ortalama_kurus liste_duzeyi yerel_pay butce_carpani donem_butce donem_ithal_carpani donem_talep_carpani'.split())
MANAGER = {'mudur_gunluk_maliyet_kurus', 'aile_mudur_gunluk_maliyet_kurus'}

def aggregate(rows, key, target):
    groups = defaultdict(list)
    for row in rows:
        groups[key(row)].append(row)
    output = []
    for _, days in groups.items():
        last = days[-1].copy()
        for col in SUM | MANAGER:
            last[col] = sum(int(d[col]) for d in days)
        for col in MEAN:
            last[col] = sum(float(d[col]) for d in days) / len(days)
        last['ilk_gun'] = days[0]['gun']
        last['oynanan_gun'] = len(days)
        last['brut_marj_yuzde'] = 100 * last['brut_kar_kurus'] / max(1, last['ciro_kurus'])
        last['raf_alis_marj_yuzde'] = 100 * (last['raf_ortalama_kurus'] - last['alis_ortalama_kurus']) / max(1, last['raf_ortalama_kurus'])
        last['donemler'] = '|'.join(dict.fromkeys(d['donem'] for d in days))
        last['mudurler'] = '|'.join(dict.fromkeys(part for d in days for part in d['mudurler'].split('|') if part))
        # Daily loss reasons count product requests, not mutually exclusive lost people.
        last['aile_mudur_patron_sonrasi_favok_kurus'] = last['aile_favok_kurus'] - last['aile_mudur_gunluk_maliyet_kurus'] - last['patron_sirket_maliyeti_kurus']
        output.append(last)
    with target.open('w', encoding='utf-8-sig', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=list(output[0]))
        writer.writeheader()
        writer.writerows(output)
    return output

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('directory', type=Path)
    args = parser.parse_args()
    with (args.directory / 'aile_gunluk.csv').open(encoding='utf-8-sig', newline='') as f:
        rows = list(csv.DictReader(f))
    assert rows and all(None not in r and None not in r.values() for r in rows), 'CSV columns do not match'
    aggregate(rows, lambda r: (r['tarz'], r['tohum'], r['takvim_yili'], r['ay']), args.directory / 'aile_aylik.csv')
    # Exact anniversary cut points supplied by calendar dates through datetime.
    from datetime import date, timedelta
    for row in rows:
        current = date(2011, 3, 7) + timedelta(days=int(row['gun']) - 1)
        row['oyun_yili'] = current.year - 2011 + (current >= date(current.year, 3, 7))
    years = aggregate(rows, lambda r: (r['tarz'], r['tohum'], r['oyun_yili']), args.directory / 'aile_yillik.csv')
    for r in years:
        print(r['tarz'], r['oyun_yili'], 'days', r['oynanan_gun'], 'sales TL', round(r['ciro_kurus']/100, 2), 'family EBITDA', round(r['aile_favok_kurus']/100, 2), 'company EBITDA', round(r['sirket_favok_kurus']/100, 2), 'cash', int(r['kasa_kurus'])/100)

if __name__ == '__main__':
    main()
