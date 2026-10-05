"""Draw review plans from actual runtime fixture coordinates, not concept art."""
import json,math
from PIL import Image,ImageDraw,ImageFont
from validate_stores import ROOT,registry
from check_store_routes import bounds
FONT='C:/Windows/Fonts/arial.ttf'
COLORS={'shelf':'#cfb892','bakery':'#e2ac65','cooler':'#8ccbd3','freezer':'#73a7d6','produce':'#9ebd72',
        'checkout':'#d78c74','self_checkout':'#e5a793','electronics':'#baacd0','home':'#baacd0',
        'textile':'#baacd0','hardware':'#baacd0','shoe':'#baacd0','service':'#e5b37b','deli':'#e5b37b',
        'butcher':'#e5b37b','fish':'#e5b37b'}
def render(s,eq):
    im=Image.new('RGB',(1200,1100),'#faf8f3');draw=ImageDraw.Draw(im)
    font=lambda n:ImageFont.truetype(FONT,n)
    draw.text((45,25),s['name'],font=font(30),fill='#203938')
    w,d=s['footprintCm'];scale=min(1090/w,780/d);cx,cy=600,520
    # Looking inward from the entrance (+Y) in Unreal, positive X is on
    # the viewer's left. Match that orientation in the review plan.
    def pt(x,y):return (cx-x*scale,cy-y*scale)
    def rect(b,fill,outline=None):
        x,y,hx,hy=b;a=pt(x-hx,y+hy);c=pt(x+hx,y-hy)
        draw.rectangle((min(a[0],c[0]),min(a[1],c[1]),max(a[0],c[0]),max(a[1],c[1])),fill=fill,outline=outline)
    draw.polygon([pt(*p) for p in s['architecture']['outlineCm']],fill='white',outline='#203938',width=3)
    lo=s['points']['backroom']['min'];hi=s['points']['backroom']['max'];rect(((lo[0]+hi[0])/2,(lo[1]+hi[1])/2,(hi[0]-lo[0])/2,(hi[1]-lo[1])/2),'#deded7')
    draw.text(pt(0,hi[1]-20),'DEPO',font=font(15),fill='#555555',anchor='mt')
    for c in s['layoutDesign']['crossAisles']:
        x0,y0=c['min'];x1,y1=c['max'];rect(((x0+x1)/2,(y0+y1)/2,(x1-x0)/2,(y1-y0)/2),'#e0eedc')
    for q in s['layoutDesign']['queues']:
        x0,y0=q['min'];x1,y1=q['max'];rect(((x0+x1)/2,(y0+y1)/2,(x1-x0)/2,(y1-y0)/2),'#fbe9df')
    for f in s['fixtures']:
        e=eq[f['equipment']];rect(bounds(f,eq),COLORS.get(e['family'],'#c7c7bb'),'#645f55')
        # Front-face marker makes outward endcaps visible in the drawing.
        a=math.radians(f['yaw']);x,y=f['at'][:2];depth=e['size'][1]/2
        draw.line([pt(x,y),pt(x+math.sin(a)*depth,y-math.cos(a)*depth)],fill='#343f44',width=2)
    for o in s['architecture'].get('obstacles',[]):rect((o['at'][0],o['at'][1],o['sizeCm'][0]/2,o['sizeCm'][1]/2),'#444444')
    x,y=s['points']['entrance']['at'][:2];p=pt(x,y+80);draw.line([pt(x,y),pt(x,y+150)],fill='#2576ab',width=6)
    draw.polygon([(p[0]-8,p[1]+10),(p[0]+8,p[1]+10),(p[0],p[1]-5)],fill='#2576ab')
    draw.text((45,940),f"{s['salesAreaM2']:g} m² satış | {len(s['fixtures'])} ekipman | {s['stats']['checkouts']} kasa",font=font(23),fill='#203938')
    for i,(label,fam) in enumerate([('Gıda / bakım','shelf'),('Manav','produce'),('Soğuk','cooler'),('Donuk','freezer'),('Fırın','bakery'),('Servis','service'),('Ev / teknoloji','home'),('Kasa','checkout')]):
        x=45+(i%4)*285;y=992+(i//4)*40;draw.rectangle((x,y,x+18,y+18),fill=COLORS[fam]);draw.text((x+27,y-2),label,font=font(19),fill='#343f44')
    return im
def main():
    doc=json.loads((ROOT/'Config/magazalar.json').read_text(encoding='utf-8'));eq=registry()
    out=ROOT/'Docs/Images/StoreLayouts/20261005';out.mkdir(parents=True,exist_ok=True)
    sheet=Image.new('RGB',(2400,2200),'white')
    for i,s in enumerate(doc['stores']):
        im=render(s,eq);im.save(out/(s['id']+'.png'));sheet.paste(im,((i%2)*1200,(i//2)*1100))
    sheet.save(out/'layouts.png');print('LAYOUT_PLANS_RENDERED=4')
if __name__=='__main__':main()
