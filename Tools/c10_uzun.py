"""Run the three C10 30-year styles in independent processes and join their CSVs.
Uses the existing commandlet -Style=0/1/2; no simulation code is changed.
"""
import concurrent.futures
import csv
import hashlib
import json
import sys
import time
from c10_kos import ROOT,run

def main():
    tune=sys.argv[1]
    dest=ROOT/'Saved/AutoPlay/C10/D6_30'
    assert not dest.exists(),'Existing long dataset must be preserved'
    started=time.monotonic()
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        jobs=[pool.submit(run,f'D6_30_style{i}',tune,30,1,i) for i in range(3)]
        parts=[job.result() for job in jobs]
    assert len({p['source'] for p in parts})==1
    folders=[ROOT/'Saved/AutoPlay/C10'/p['experiment'] for p in parts]
    names={p.name for p in folders[0].glob('*.csv')}
    assert all({p.name for p in folder.glob('*.csv')}==names for folder in folders)
    dest.mkdir()
    evidence={}
    for name in sorted(names):
        header=None
        with (dest/name).open('w',encoding='utf-8-sig',newline='') as output:
            writer=csv.writer(output)
            for folder in folders:
                path=folder/name
                evidence[str(path.relative_to(ROOT))]=hashlib.sha256(path.read_bytes()).hexdigest()
                with path.open(encoding='utf-8-sig',newline='') as input_file:
                    reader=csv.reader(input_file);current=next(reader)
                    if header is None:header=current;writer.writerow(header)
                    assert current==header,(name,'schema mismatch')
                    writer.writerows(reader)
    report='\n\n'.join((folder/'rapor.md').read_text(encoding='utf-8-sig') for folder in folders)
    (dest/'rapor.md').write_text(report,encoding='utf-8')
    meta={'experiment':'D6_30','tune':tune,'years':30,'seeds':1,'first_seed':21,
          'source':parts[0]['source'],'exit_code':0,'seconds':round(time.monotonic()-started,2),
          'execution':'three independent existing -Style commandlets; CSV headers joined once',
          'parts':parts,'part_csv_sha256':evidence,
          'excluded_sequential_attempt':'D6_30_sequential_unused'}
    (dest/'manifest.json').write_text(json.dumps(meta,indent=2),encoding='utf-8')
    print(json.dumps({'experiment':'D6_30','seconds':meta['seconds'],'parts':len(parts)},indent=2),flush=True)

if __name__=='__main__':main()
