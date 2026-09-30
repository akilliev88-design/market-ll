"""Blender 5.2: fabricate Phase A equipment and shells using the established kit helpers."""
import sys, json, math
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import create_store_kit as kit
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from store_specs import EQUIPMENT
import bpy

ROOT=kit.ROOT
kit.BASE=ROOT/'AssetInbox/Environment/Stores'
kit.PREVIEWS=ROOT/'Docs/Images/Stores'
original_preview=kit.preview
def preview(path,model,camera,target):
    original_preview(path,model,camera,target)
kit.preview=preview

def equipment(e,s):
    kit.reset()
    metal=kit.mat('MI_Store_PaintedMetal',(.68,.70,.67),.35,.38)
    dark=kit.mat('MI_Store_DarkMetal',(.025,.035,.037),.7,.28)
    wood=kit.mat('MI_Store_WarmWood',(.32,.14,.055),0,.42)
    rail=kit.mat('MI_Store_PriceRail',(.52,.055,.025),.05,.4)
    white=kit.mat('MI_Store_White',(.82,.82,.78),0,.36)
    glass=kit.mat('MI_Store_Glass',(.35,.48,.50),.05,.13,.75)
    light=kit.mat('MI_Store_Light',(.85,.86,.80),0,.2,emission=(.85,.86,.8))
    w,d,h=[v/100 for v in s['size']]; f=s['family']; parts=[]
    def box(label,p,size,mat=metal,bevel=.005):
        obj=kit.cube(label,p,size,mat,bevel); parts.append(obj); return obj
    if f=='checkout':
        for lane in range(s.get('checkouts',1)):
            x=(lane-(s.get('checkouts',1)-1)/2)*1.1
            box('Cabinet',(x,0,.43),(1.05,d,.86),metal,.02)
            box('Belt',(x,-.25,.9),(.62,1.65,.07),dark)
            for sign in (-1,1): box('BeltEdge',(x+sign*.37,-.25,.94),(.04,1.65,.06),white)
            box('Scanner',(x,.65,.96),(.30,.27,.08),dark)
            box('Till',(x+.27,.65,1.06),(.23,.22,.12),dark,.012)
            box('BagWell',(x,-1.0,.81),(.7,.45,.04),dark)
    elif f=='cooler':
        box('Back',(0,d/2-.04,h/2),(w,.08,h),metal)
        box('Base',(0,0,.12),(w,d,.24),dark)
        box('Top',(0,0,h-.12),(w,d,.24),white)
        for x in (-w/2+.025,w/2-.025): box('Side',(x,0,h/2),(.05,d,h),metal)
        for z in s['levels']: box('Shelf',(0,0,z/100-.015),(w-.08,d-.15,.03),white)
        for n in range(3):
            x=(n-1)*w/3
            box('GlassDoor',(x,-d/2+.045,h/2),(w/3-.045,.015,h-.45),glass,.002)
            box('DoorFrame',(x-w/6+.015,-d/2+.02,h/2),(.03,.05,h-.3),dark)
            box('Handle',(x+w/6-.085,-d/2-.012,h*.5),(.02,.025,.38),dark)
            box('Led',(x,-d/2+.11,h-.3),(w/3-.12,.02,.025),light)
    elif f in ('freezer','deli','fish','butcher'):
        box('Base',(0,.02,.30),(w,d,.60),white,.02)
        for x in (-w/2+.04,w/2-.04): box('End',(x,0,h/2),(.08,d,h),metal)
        box('DisplayBed',(0,0,s['levels'][0]/100-.025),(w-.16,d-.18,.05),dark)
        box('FrontGlass',(0,-d/2+.06,h*.72),(w-.16,.02,h*.46),glass)
        box('TopGlass',(0,0,h-.04),(w-.16,d-.12,.02),glass)
        box('Rail',(0,-d/2-.005,h*.5),(w-.10,.02,.05),rail)
    elif f in ('bakery','tobacco','home','electronics','textile'):
        box('Back',(0,d/2-.025,h/2),(w,.05,h),wood if f=='bakery' else metal)
        for x in (-w/2+.025,w/2-.025): box('Post',(x,d/2-.06,h/2),(.05,.08,h),dark)
        for z in s['levels'] or [35,85,135,180]:
            box('Board',(0,0,z/100-.02),(w-.08,d,.04),wood if f=='bakery' else white)
            box('Rail',(0,-d/2,z/100-.04),(w-.08,.02,.05),rail)
        if f=='electronics':
            for x in (-.65,0,.65):
                box('Screen',(x,.06,1.4),(.48,.06,.3),dark)
                box('ScreenStand',(x,.06,1.2),(.08,.10,.14),metal)
        if f=='textile':
            for x in (-.65,0,.65): box('FoldedFabric',(x,0,.94),(.4,.35,.18),wood,.02)
        if f=='home':
            for x in (-.65,0,.65): box('Housewares',(x,0,.97),(.30,.30,.24),white,.03)
    elif f=='produce':
        box('Plinth',(0,0,.27),(w,d,.54),wood,.015)
        box('Tray',(0,0,.66),(w-.08,d-.08,.09),dark)
        for n in range(max(2,int(w/.5))):
            x=-w/2+.28+n*.5
            box('Crate',(x,0,.78),(.46,d-.13,.20),wood)
            for row in range(3):
                for col in range(3):
                    fruit=kit.cylinder('Produce',(x+(col-1)*.11,(row-1)*.18,.91),.055,.065,rail if n%2==0 else wood,vertices=12)
                    parts.append(fruit)
    elif f=='pallet':
        for n in range(6): box('PalletSlat',(-w/2+.09+n*.20,0,.12),(.16,d,.04),wood)
        for x in (-.4,.4): box('PalletFoot',(x,0,.06),(.14,d,.12),wood)
        for x in (-.3,.3):
            for y in (-.23,.23): box('Case',(x,y,.36),(.54,.42,.44),wood,.01)
    elif f=='basket':
        for n in range(9):
            z=.22+n*.07
            box('BasketBase',(0,0,z),(.46,.31,.018),dark)
            for x in (-.23,.23): box('BasketSide',(x,0,z+.035),(.014,.31,.075),rail)
            for y in (-.16,.16): box('BasketSide',(0,y,z+.035),(.46,.014,.075),rail)
    elif f=='cart':
        for n in range(4):
            x=-1.1+n*.65
            box('CartBed',(x,0,.58),(.50,.75,.025),metal)
            for y in (-.375,.375): box('CartEnd',(x,y,.76),(.5,.025,.35),metal)
            for xx in (-.25,.25):
                for bar in range(7): box('Wire',(x+xx,-.3+bar*.1,.76),(.012,.012,.36),metal,.001)
                for y in (-.24,.24):
                    parts.append(kit.cylinder('Wheel',(x+xx,y,.10),.09,.04,dark,rotation=(0,math.pi/2,0),vertices=12))
            box('Handle',(x,.40,.99),(.54,.035,.035),rail)
    else:
        box('ServiceDesk',(0,0,.47),(w,d,.94),metal,.02)
        box('DeskTop',(0,0,.98),(w+.03,d+.03,.07),wood)
    name='SM_'+s['name']; model=kit.join(parts,name,e,tuple(v*10 for v in s['size']))
    collisions=[kit.collision(name,0,(0,0,.25),(w,d,.50))]
    # Separate spine/end blockers keep placement regions open.
    if f in ('bakery','tobacco','cooler'): collisions.append(kit.collision(name,1,(0,d/2-.05,h/2),(w,.10,h)))
    points=[obj.matrix_world @ kit.Vector(p) for obj in [model,*collisions] for p in obj.bound_box]
    measured=[round((max(p[i] for p in points)-min(p[i] for p in points))*1000,2) for i in range(3)]
    model['dimensions_mm']=','.join(str(v) for v in measured)
    zones=[dict(id=f'front_l{i:02d}',face='front',level=i,centerCm=[0,0,z],usableWidthCm=s['size'][0]-8,usableDepthCm=s['size'][1]-15,clearanceHeightCm=(s['levels'][i+1]-z-5 if i+1<len(s['levels']) else max(25,s['size'][2]-z-5))) for i,z in enumerate(s['levels'])]
    metadata=dict(schemaVersion=1,id=e,mesh=name+'.fbx',unrealMesh=f'/Game/Stores/Equipment/{name}.{name}',family=f,
        dimensionsMm=dict(zip(['width','depth','height'],measured)),origin='floor_center',frontAxis='-Y',
        materials=[m.name for m in model.data.materials],collision=dict(policy='custom_ucx',pieces=len(collisions)),zones=zones,
        checkouts=s.get('checkouts',0),planogram=dict(doubleSided=False,widthCm=s['size'][0]-8,depthCm=s['size'][1]-15,frontY=-s['size'][1]/2+8,meshYaw=180),
        lod=dict(naniteCandidate=True,lod1TriangleRatio=.5,lod2TriangleRatio=.2))
    kit.save_asset(e,name,model,collisions,metadata,(w*1.3,-d*3,h*1.2),(0,0,h/2))

def shell(s):
    kit.reset(); w,d=[v/100 for v in s['footprintCm']]; h=s['ceilingCm']/100; back=s['backroomM2']/w; rear=d/2-back
    floor=kit.mat('MI_Store_Floor',(.42,.40,.36) if s['theme']=='sicak_ahsap' else (.64,.67,.64),0,.55)
    wall=kit.mat('MI_Store_Wall',(.79,.78,.70),0,.63)
    dark=kit.mat('MI_Store_DarkMetal',(.025,.035,.037),.7,.28)
    light=kit.mat('MI_Store_Light',(.85,.86,.8),0,.2,emission=(.9,.87,.8))
    glass=kit.mat('MI_Store_FacadeGlass',(.45,.58,.62),.05,.12,.85)
    parts=[]; collisions=[]; name='SM_Shell_'+s['id']
    def box(label,p,size,mat=wall,block=True):
        # With UE's FBX Y mirror and yaw=180, author X opposite to the UE contract.
        p=(-p[0],p[1],p[2]); parts.append(kit.cube(label,p,size,mat,.004))
        if block: collisions.append(kit.collision(name,len(collisions),p,size))
    box('Floor',(0,0,.025),(w,d,.05),floor)
    for x in (-w/2,w/2): box('Side',(x,0,h/2),(.12,d,h))
    # Front and rear doors are real holes, including the storage-room passage.
    for y,gapx,gap in [(-d/2,0,2.4),(d/2,w/2-1.5,1.6),(rear,w/2-1.5,1.6)]:
        left=-w/2; a=gapx-gap/2; b=gapx+gap/2; right=w/2
        for x0,x1 in [(left,a),(b,right)]:
            if x1>x0:
                if y==-d/2:
                    box('Sill',((x0+x1)/2,y,.225),(x1-x0,.12,.45))
                    box('Window',((x0+x1)/2,y,1.375),(x1-x0,.025,1.85),glass)
                    box('Fascia',((x0+x1)/2,y,(h+2.3)/2),(x1-x0,.12,h-2.3))
                    for xx in (x0,x1): box('WindowMullion',(xx,y,1.375),(.04,.06,1.85),dark,False)
                else: box('Wall',( (x0+x1)/2,y,h/2),(x1-x0,.12,h))
        box('Lintel',(gapx,y,(h+2.3)/2),(gap,.12,h-2.3))
    box('Roof',(0,0,h-.025),(w,d,.05))
    # Visible service beams and strips below the roof.
    for x in range(int(-w/2+2),int(w/2),4):
        box('Beam',(x,0,h-.08),(.10,d,.16),dark,False)
        for y in range(int(-d/2+2),int(d/2),4):
            box('Luminaire',(x,y,h-.23),(.14,1.3,.08),dark,False)
            box('Diffuser',(x,y,h-.28),(.12,1.28,.02),light,False)
    model=kit.join(parts,name,s['id'],(int(w*1000),int(d*1000),int(h*1000)))
    kit.save_asset('shell_'+s['id'],name,model,collisions,dict(schemaVersion=1,id=s['id'],mesh=name+'.fbx',origin='floor_center',frontAxis='-Y',dimensionsMm=dict(width=w*1000,depth=d*1000,height=h*1000),collision=dict(policy='custom_ucx',pieces=len(collisions))), (w*.7,-d*.9,h*1.5),(0,0,1))

if __name__=='__main__':
    for e,s in EQUIPMENT.items(): equipment(e,s)
    for s in json.loads((ROOT/'Config/magazalar.json').read_text(encoding='utf-8'))['stores']: shell(s)
    from export_store_fbx import export_all
    export_all(ROOT)
    print('MIRAS_PHASE_A_MODELS_READY')
