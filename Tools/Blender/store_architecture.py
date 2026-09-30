"""Build authored orthogonal shells, recesses, structural columns and department finishes."""
import math

def inside(x,y,points):
    hit=False
    for a,b in zip(points,points[1:]+points[:1]):
        if (a[1]>y)!=(b[1]>y) and x<(b[0]-a[0])*(y-a[1])/(b[1]-a[1])+a[0]:hit=not hit
    return hit

def build_shell(kit,s):
    kit.reset()
    w,d=[v/100 for v in s['footprintCm']]; h=s['ceilingCm']/100
    outline=[[x/100,y/100] for x,y in s['architecture']['outlineCm']]
    warm=s['theme']=='sicak_ahsap'; green=s['theme']=='dogal_yesil'
    prefix=s['id']
    floor=kit.mat('MI_'+prefix+'_Floor',(.56,.54,.49) if warm else (.65,.66,.63),0,.52)
    wall=kit.mat('MI_'+prefix+'_Wall',(.74,.71,.65) if warm else (.78,.79,.75),0,.62)
    wood=kit.mat('MI_'+prefix+'_Oak',(.32,.24,.15),0,.43)
    accent=kit.mat('MI_'+prefix+'_Accent',(.12,.22,.16) if green else (.12,.17,.19),0,.43)
    dark=kit.mat('MI_'+prefix+'_Steel',(.045,.055,.055),.65,.34)
    light=kit.mat('MI_'+prefix+'_Light',(.83,.84,.79),0,.22,emission=(.9,.88,.82))
    glass=kit.mat('MI_'+prefix+'_Glass',(.36,.47,.48),.1,.12,.85)
    finishes={'wood':kit.mat('MI_'+prefix+'_FreshFloor',(.40,.30,.19),0,.52),
              'entry':kit.mat('MI_'+prefix+'_EntryFloor',(.27,.29,.28),0,.52),
              'bulk':kit.mat('MI_'+prefix+'_BulkFloor',(.43,.44,.40),0,.6),
              'household':kit.mat('MI_'+prefix+'_CareFloor',(.48,.56,.52),0,.5)}
    name='SM_Shell_'+prefix; parts=[]; collisions=[]
    def box(label,p,size,mat=wall,block=True):
        # The legacy importer mirrors Y; yaw=180 then restores the contract with authored X reversed.
        p=(-p[0],p[1],p[2]);parts.append(kit.cube(label,p,size,mat,.004))
        if block:collisions.append(kit.collision(name,len(collisions),p,size))
    xs=sorted({p[0] for p in outline});ys=sorted({p[1] for p in outline})
    cells=[]
    for x0,x1 in zip(xs,xs[1:]):
        for y0,y1 in zip(ys,ys[1:]):
            if inside((x0+x1)/2,(y0+y1)/2,outline):
                cells.append((x0,x1,y0,y1))
                box('Floor',((x0+x1)/2,(y0+y1)/2,.025),(x1-x0,y1-y0,.05),floor)
    for zone in s['architecture']['zones']:
        x,y=[v/100 for v in zone['at']];zw,zd=[v/100 for v in zone['sizeCm']]
        # Clip floor finishes to the true outline; no hidden surface fills the recessed corner.
        for x0,x1,y0,y1 in cells:
            a,b=max(x-zw/2,x0),min(x+zw/2,x1);c,e=max(y-zd/2,y0),min(y+zd/2,y1)
            if b>a and e>c:box('DepartmentFloor',((a+b)/2,(c+e)/2,.051),(b-a,e-c,.002),finishes[zone['finish']],False)
    for a,b in zip(outline,outline[1:]+outline[:1]):
        horizontal=abs(a[1]-b[1])<.01
        pos=a[1] if horizontal else a[0]
        lo,hi=sorted([a[0],b[0]] if horizontal else [a[1],b[1]])
        is_front=horizontal and abs(pos+d/2)<.01
        is_back=horizontal and abs(pos-d/2)<.01
        door=(s['points']['entrance']['at'][0] if is_front else s['points']['receiving']['at'][0])/100
        gap=2.4 if s['format']=='mahalle' else 3.4 if s['format']!='hiper' else 5.0
        if is_back:gap=1.6
        spans=[(lo,door-gap/2),(door+gap/2,hi)] if is_front or is_back else [(lo,hi)]
        for u,v in spans:
            if v<=u:continue
            xy=((u+v)/2,pos) if horizontal else (pos,(u+v)/2)
            size=(v-u,.14,h) if horizontal else (.14,v-u,h)
            if is_front:
                box('FrontSill',(*xy,.225),(v-u,.14,.45))
                box('ShopWindow',(*xy,1.35),(v-u,.025,1.8),glass)
                box('Fascia',(*xy,(h+2.25)/2),(v-u,.14,h-2.25),accent)
                for n in range(max(1,math.ceil((v-u)/2.5))+1):
                    xx=u+(v-u)*n/max(1,math.ceil((v-u)/2.5))
                    box('WindowMullion',(xx,pos,1.35),(.045,.06,1.8),dark,False)
            else:
                box('Wall',(*xy,h/2),size)
                # A low dado and timber/painted accents give small shops a different envelope.
                box('Dado',(*xy,.40),(size[0]+.01,size[1]+.01,.7),wood if warm else accent,False)
        if is_front or is_back:box('DoorLintel',(door,pos,(h+2.25)/2),(gap,.14,h-2.25),accent if is_front else wall)
    rear=s['points']['backroom']['min'][1]/100;door=s['points']['receiving']['at'][0]/100
    for a,b in [(-w/2,door-.8),(door+.8,w/2)]:box('BackroomWall',((a+b)/2,rear,h/2),(b-a,.14,h))
    box('ReceivingLintel',(door,rear,(h+2.25)/2),(1.6,.14,h-2.25))
    for ob in s['architecture']['obstacles']:
        x,y,_=[v/100 for v in ob['at']];ww,dd,hh=[v/100 for v in ob['sizeCm']]
        box(ob['id'],(x,y,hh/2),(ww,dd,hh))
        if ob['kind']=='column':
            box('ColumnBase',(x,y,.15),(ww+.025,dd+.025,.3),dark,False)
            box('ColumnCladding',(x,y,.70),(ww+.012,dd+.012,1.1),wood if warm else accent,False)
    # Shared commercial ceiling language; keep the roof separate for inspectable cutaway views.
    step=4 if w<45 else 6
    for y in range(int(-d/2+2),int(d/2),step):
        for x in range(int(-w/2+2),int(w/2),step):
            if inside(x,y,outline):
                box('Luminaire',(x,y,h-.23),(1.5,.17,.08),dark,False)
                box('Diffuser',(x,y,h-.28),(1.46,.14,.02),light,False)
    model=kit.join(parts,name,prefix,(int(w*1000),int(d*1000),int(h*1000)))
    metadata=dict(schemaVersion=1,id=prefix,mesh=name+'.fbx',origin='floor_center',frontAxis='-Y',dimensionsMm=dict(width=w*1000,depth=d*1000,height=h*1000),collision=dict(policy='custom_ucx',pieces=len(collisions)))
    kit.save_asset('shell_'+prefix,name,model,collisions,metadata,(w*.6,-d*.8,h*1.5),(0,0,1))
    kit.reset();roof_parts=[];roof_ucx=[];roof_name='SM_Roof_'+prefix
    for i,(x0,x1,y0,y1) in enumerate(cells):
        p=(-(x0+x1)/2,(y0+y1)/2,h+.035);size=(x1-x0,y1-y0,.07)
        roof_parts.append(kit.cube('RoofPanel',p,size,kit.mat('MI_'+prefix+'_Roof',(.36,.38,.37),.3,.5),.004))
        roof_ucx.append(kit.collision(roof_name,i,p,size))
    roof=kit.join(roof_parts,roof_name,'roof_'+prefix,(int(w*1000),int(d*1000),int((h+.07)*1000)))
    kit.save_asset('roof_'+prefix,roof_name,roof,roof_ucx,dict(schemaVersion=1,id='roof_'+prefix,mesh=roof_name+'.fbx',origin='floor_center',frontAxis='-Y',dimensionsMm=dict(width=w*1000,depth=d*1000,height=(h+.07)*1000),collision=dict(policy='custom_ucx',pieces=len(roof_ucx))),(w*.6,-d*.8,h*1.5),(0,0,h))
