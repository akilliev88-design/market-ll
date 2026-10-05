"""Local border variance, rather than whole-screen pass/fail percentages."""
import sys,json
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1]
labels=sys.argv[1:] or ['replacement']
out=root/'Docs/Images/StoreEdges/20261005';out.mkdir(parents=True,exist_ok=True)
# Diagnostic screen regions at one camera position. A replacement has different
# geometry: these values are local noise diagnostics, not an equal-edge benchmark.
regions={'upper_frame':[370,65,1330,265],'right_post':[1230,120,1300,845],'base_corner':[340,770,460,865],'door_post':[875,160,915,860]}
result=[]
for label in labels:
    folder=root/'Saved/Screenshots/StoreEdges'/label
    images=[Image.open(folder/f'{i:03}.png').convert('RGB') for i in range(10,40)]
    row={'label':label,'samples':len(images),'regions':{}}
    for name,box in regions.items():
        a=np.asarray([np.asarray(im.crop(box),dtype=np.float32) for im in images])
        diff=np.abs(np.diff(a,axis=0));variation=np.std(a,axis=0).mean(axis=2)
        row['regions'][name]={'meanTemporalStd':float(variation.mean()),'p95TemporalStd':float(np.percentile(variation,95)),'meanAdjacentRGBDiff':float(diff.mean()),'percentPixelsStdAbove5':float((variation>5).mean()*100)}
    result.append(row)
    im=images[10].copy();draw=ImageDraw.Draw(im)
    for name,box in regions.items():draw.rectangle(box,outline='red',width=3);draw.text((box[0]+3,box[1]+3),name,fill='red')
    im.save(out/(label+'_regions.png'))
    frames=[Image.open(folder/f'{i:03}.png').convert('RGB').resize((960,540)) for i in range(40,80)]
    frames[0].save(out/(label+'_moving.gif'),save_all=True,append_images=frames[1:],duration=65,loop=0)
(out/'edge-variance.json').write_text(json.dumps(result,indent=2))
for row in result:print(json.dumps(row))
