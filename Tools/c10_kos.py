"""Run isolated C10 commandlets with fixed seeds, explicit knobs and unique evidence paths.
Usage: python Tools/c10_kos.py singles | combined "Key=Value,..." | long "Key=Value,..."
Each experiment is an independent process; no player save is written.
"""
import concurrent.futures
import json
import subprocess
import sys
import time
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
ENGINE=Path('C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe')
SINGLES=[('D0',''),('D1','BranchCompetition=2.5'),('D2','RealWageGrowth=0.005'),
         ('D3','RealSpend=0.7'),('D4','OpenBuffer.Balanced=1.0'),
         ('D4b','OpenBuffer.Careful=2.0'),('D5','LossMonthsToClose=4')]

def run(case,tune,years=10,seeds=3,style=None):
    target=ROOT/'Saved/AutoPlay/C10'/case
    target.mkdir(parents=True,exist_ok=True)
    assert not (target/'manifest.json').exists(),f'Existing experiment must not be overwritten: {target}'
    source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
    log=ROOT/'Saved/Logs'/f'C10_{case}.log'
    command=[str(ENGINE),str(ROOT/'MarketSim.uproject'),'-run=MarketSimAutoPlay',
             f'-Years={years}',f'-Seeds={seeds}','-Seed=21',f'-Experiment={case}',
             '-unattended','-nop4','-nosound','-NullRHI',f'-abslog={log}']
    if style is not None:command.append(f'-Style={style}')
    if tune:command.append(f'-Tune={tune}')
    meta={'experiment':case,'tune':tune,'years':years,'seeds':seeds,'first_seed':21,
          'source':source,'style':style,'command':command,'started_utc':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime())}
    (target/'manifest.json').write_text(json.dumps(meta,indent=2),encoding='utf-8')
    started=time.monotonic()
    with (target/'console.log').open('w',encoding='utf-8') as output:
        process=subprocess.Popen(command,cwd=ROOT,stdout=output,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW)
        while process.poll() is None:
            time.sleep(20)
            lines=log.read_text(encoding='utf-8-sig',errors='replace').splitlines() if log.exists() else []
            progress=next((line for line in reversed(lines) if 'AutoPlay progress' in line or 'audit failures' in line),'starting')
            tuned=next((line.split('AutoPlay Tune: ')[1].strip() for line in lines if 'AutoPlay Tune: ' in line),None)
            if tuned is not None:
                actual={} if tuned=='C10 defaults' else {k:float(v) for k,v in (part.split('=') for part in tuned.split(', '))}
                expected={k:float(v) for k,v in (part.split('=') for part in tune.split(','))} if tune else {}
                if actual!=expected:
                    process.terminate();process.wait()
                    raise RuntimeError(('Commandlet applied different Tune',case,actual,expected))
            print(f'{case} {time.monotonic()-started:.0f}s {progress}',flush=True)
    meta.update(exit_code=process.returncode,seconds=round(time.monotonic()-started,2))
    (target/'manifest.json').write_text(json.dumps(meta,indent=2),encoding='utf-8')
    assert process.returncode==0,(case,process.returncode,log)
    assert (target/'gunluk.csv').exists(),(case,'missing report')
    print(f'{case} COMPLETE {meta["seconds"]}s',flush=True)
    return meta

if __name__=='__main__':
    mode=sys.argv[1]
    if mode=='singles':
        with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
            futures=[pool.submit(run,*case) for case in SINGLES]
            results=[f.result() for f in futures]
    elif mode=='combined':results=[run('D6',sys.argv[2])]
    elif mode=='long':results=[run('D6_30',sys.argv[2],30,1)]
    else:raise ValueError('singles / combined / long')
    print(json.dumps(results,indent=2),flush=True)
