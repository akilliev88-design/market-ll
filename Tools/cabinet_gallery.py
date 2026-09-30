"""Publish the actual Blender renders as a local review sheet and browseable gallery."""
from pathlib import Path
import json,html,shutil
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[1];out=root/'Docs/Images/Stores/Cabinets';out.mkdir(parents=True,exist_ok=True)
font=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',19)
entries=[];tiles=[]
for p in sorted((root/'AssetInbox/Environment/Stores').rglob('equipment.json')):
    m=json.loads(p.read_text(encoding='utf-8'))
    if 'reference' not in m:continue
    image=p.parent/'preview.png';target=out/(m['id']+'.png');shutil.copy2(image,target)
    tile=Image.new('RGB',(400,445),'#e8edf0');im=Image.open(image).convert('RGB');im.thumbnail((400,400));tile.paste(im,((400-im.width)//2,0));ImageDraw.Draw(tile).text((12,411),m['displayName'],font=font,fill='#243341');tiles.append(tile)
    dims=m['dimensionsMm'];entries.append(f'<article><img src="{m["id"]}.png"><h2>{html.escape(m["displayName"])}</h2><p>{dims["width"]/1000:.2f} × {dims["depth"]/1000:.2f} × {dims["height"]/1000:.2f} m</p><a href="../../../../AssetInbox/Environment/Stores/{m["id"]}/Source/{m["id"]}.blend">Blender dosyası</a></article>')
sheet=Image.new('RGB',(1600,445*((len(tiles)+3)//4)),'#e8edf0')
for i,tile in enumerate(tiles):sheet.paste(tile,((i%4)*400,(i//4)*445))
sheet.save(out/'cabinet_library.jpg',quality=94)
(out/'index.html').write_text('<!doctype html><meta charset="utf-8"><title>Miras Market — Dolap Kütüphanesi</title><style>body{font:16px system-ui;background:#edf1f4;color:#243341;margin:32px}main{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:24px}article{background:white;padding:16px;border-radius:12px}img{width:100%}h2{font-size:18px}a{color:#225a83}</style><h1>Dolap ve teşhir kütüphanesi</h1><p>Gerçek Blender renderları. Ölçüler oyun için tasarlanmıştır; üretici teknik ölçüleri değildir.</p><main>'+''.join(entries)+'</main>',encoding='utf-8')
print('CABINET_GALLERY',len(tiles),out)
