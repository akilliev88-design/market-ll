"""Compare repeated fixed-camera renders; report camera pans separately."""
import json
from pathlib import Path
import numpy as np
from PIL import Image
root=Path(__file__).resolve().parents[1]
reports=[]
for sid in ('kucuk_01','buyuk_01','hiper_01'):
    folder=root/'Saved/Screenshots/LargeStores'/sid
    arrays=[np.asarray(Image.open(folder/f'{i:02}.png').convert('RGB'),dtype=np.float32) for i in (4,5,6)]
    pairs=[]
    for a,b in zip(arrays,arrays[1:]):
        diff=np.abs(a-b);pairs.append(dict(meanAbsoluteRGB=float(diff.mean()),percentPixelsAbove20=float((diff.max(axis=2)>20).mean()*100)))
    passed=all(p['percentPixelsAbove20']<2 for p in pairs)
    reports.append(dict(store=sid,fixedCameraPairs=pairs,passed=passed,threshold='<2% of pixels change more than 20/255',movingCameraFrames=['07.png','08.png'],scope='Rendered appearance stability; not a proof against all temporal artifacts'))
target=root/'Docs/Images/LargeStores/20261005/temporal-review.json';target.parent.mkdir(parents=True,exist_ok=True)
target.write_text(json.dumps(reports,indent=2),encoding='utf-8')
for r in reports:print(r)
if not all(r['passed'] for r in reports):raise SystemExit(1)
