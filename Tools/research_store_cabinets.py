"""Read the user's manufacturer reference, retaining URLs and reference photos for modelling."""
from pathlib import Path
import re, json, urllib.request, html
from PIL import Image, ImageDraw
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'Saved/BuzReference'; OUT.mkdir(parents=True,exist_ok=True)
BASE='http://www.buzrefrigeration.com'
groups=['dikey-reyonlar','alcak-dikey-reyonlar','servis-reyonlari','dikey-dondurucular','havuz-tipi-dondurucular','icten-motorlu-sutlukler','pasta-ozel-teshir-reyonlari','dondurma-reyonlari','tek-kapakli-icecek-dolaplari','2-3-kapakli-icecek-dolaplari','kutu-tipi-sogutucular']
records=[]; tiles=[]
for group in groups:
    url=BASE+'/tr/urunler/'+group
    try:
        text=urllib.request.urlopen(url,timeout=20).read().decode('utf-8')
        (OUT/(group+'.html')).write_text(text,encoding='utf-8')
        images=list(dict.fromkeys(re.findall(r'<img[^>]+src="([^"]*uploadedfiles/product[^"]+)"',text)))
        links=list(dict.fromkeys(re.findall(r'href="([^"]*/tr/urunler/[^"?]+)"',text)))
        records.append(dict(category=group,url=url,images=images,links=links))
        for n,src in enumerate(images[:2]):
            path=OUT/(group+str(n)+'.jpg'); path.write_bytes(urllib.request.urlopen(BASE+src,timeout=20).read())
            im=Image.open(path).convert('RGB'); im.thumbnail((380,245))
            tile=Image.new('RGB',(400,280),'white');tile.paste(im,((400-im.width)//2,0));ImageDraw.Draw(tile).text((8,255),group,fill='black');tiles.append(tile)
        print(group,len(images),flush=True)
    except Exception as e: print(group,str(e),flush=True)
(OUT/'sources.json').write_text(json.dumps(records,ensure_ascii=False,indent=2),encoding='utf-8')
sheet=Image.new('RGB',(1200,280*((len(tiles)+2)//3)),(235,235,235))
for i,tile in enumerate(tiles):sheet.paste(tile,((i%3)*400,(i//3)*280))
sheet.save(OUT/'reference_sheet.jpg')
