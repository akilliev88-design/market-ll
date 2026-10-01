"""Preserve compact C8 evidence from three verified campaign directories."""
import argparse
import csv
import json
import shutil
from collections import defaultdict
from datetime import date, timedelta
from pathlib import Path
from statistics import median

def read(path):
    with path.open(encoding='utf-8-sig', newline='') as f:
        return list(csv.DictReader(f))

def write(path, rows):
    assert rows
    with path.open('w', encoding='utf-8-sig', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)

def plans(rows):
    groups = defaultdict(list)
    for row in rows:
        if int(row['tohum']) == 21:
            groups[row['tarz']].append(row)
    output = {}
    for style, records in groups.items():
        days = sorted(int(r['gun']) for r in records)
        gaps = [b-a for a, b in zip(days, days[1:])]
        output[style] = {'plan': len(days), 'ilk': days[0], 'yenileme': sum(int(r['yeni_plan_gun']) > 0 for r in records),
                         'ortanca_ara': median(gaps) if gaps else None, 'en_kisa': min(gaps) if gaps else None,
                         'en_uzun': max(gaps) if gaps else None, 'ilk_yeni_sube': min((int(r['ilk_yeni_sube']) for r in records if int(r['ilk_yeni_sube']) > 0), default=None)}
    return output

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('output', type=Path)
    parser.add_argument('standard', type=Path)
    parser.add_argument('single', type=Path)
    parser.add_argument('ten', type=Path)
    parser.add_argument('c7', type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    stats = {'source': '9c45f93', 'paths': {name: str(getattr(args, name).resolve()) for name in ('standard','single','ten','c7')}}
    for name, folder in [('normal',args.standard), ('tek_dukkan',args.single)]:
        daily = read(folder/'aile_gunluk.csv')
        assert len(daily) == 2922
        for suffix in ('aylik','yillik','urunler'):
            shutil.copyfile(folder/f'aile_{suffix}.csv', args.output/f'{name}_{suffix}.csv')
        events = read(folder/'aile_kararlar.csv')
        write(args.output/f'{name}_kadro_ve_patron.csv', [r for r in events if r['tur'] != 'order'])
        orders = defaultdict(lambda: {'adet':0, 'kurus':0, 'urunler':defaultdict(int)})
        for event in events:
            if event['tur'] != 'order': continue
            day = date(2011,3,7)+timedelta(days=int(event['gun'])-1)
            order = orders[(day.year,day.month)]
            order['adet'] += 1; order['kurus'] += int(event['deger'])
            for line in event['aciklama'].split('|'):
                if line:
                    product, cases = line.rsplit(':',1)
                    order['urunler'][product] += int(cases)
        write(args.output/f'{name}_siparisler.csv', [{'takvim_yili':year, 'ay':month, 'siparis':v['adet'], 'bedel_kurus':v['kurus'], 'urun_koli': '|'.join(f'{k}:{v["urunler"][k]}' for k in sorted(v['urunler']))} for (year,month),v in orders.items()])
        audit = read(folder/'c3.csv')
        assert all(int(r['fark_gun']) == int(r['fark_kurus']) == int(r['mutlak_fark_kurus']) == 0 for r in audit)
        stats[name] = {'days':len(daily), 'months':len(read(folder/'aile_aylik.csv')), 'rescue':plans(read(folder/'kurtarma.csv')), 'audit':audit}
        if name == 'tek_dukkan':
            assert all(int(r['magaza']) == 1 for r in daily)
            assert not read(folder/'kurtarma.csv')
    standard = read(args.standard/'gunluk.csv')
    ten = [r for r in read(args.ten/'gunluk.csv') if r['tarz']=='Temkinli' and int(r['gun'])<=2922]
    assert standard == ten, 'Independent eight-year run disagrees with ten-year prefix'
    assert standard[:366] == [{**r,'tarz':'Temkinli'} for r in read(args.single/'gunluk.csv')[:366]], 'Paired runs differ before expansion'
    stats['C8'] = plans(read(args.ten/'kurtarma.csv'))
    stats['C7'] = plans(read(args.c7/'kurtarma.csv'))
    shutil.copyfile(args.ten/'kurtarma.csv',args.output/'C8_kurtarma.csv')
    shutil.copyfile(args.ten/'aile_yillik.csv',args.output/'uc_tarz_yillik.csv')
    ad = defaultdict(lambda: [0,0])
    for row in read(args.ten/'reklam.csv'):
        ad[row['tarz']][0] += int(row['harcama_kurus']); ad[row['tarz']][1] += int(row['tahmini_magaza_ek_ciro_kurus'])
    stats['reklam'] = {style:{'harcama_kurus':v[0], 'tahmini_ciro_kurus':v[1], 'harcama_ciro':v[0]/v[1] if v[1] else None} for style,v in ad.items()}
    stats['eslesme'] = 'standard8 == careful10 prefix; both styles same first 366 days; all stock/cash audits zero'
    (args.output/'olcum.json').write_text(json.dumps(stats,ensure_ascii=False,indent=2),encoding='utf-8')
    print(json.dumps(stats,ensure_ascii=False,indent=2))

if __name__ == '__main__':
    main()
