"""Manufacturer-reference display cabinet library; metre sources, centimetre FBX copies."""
import sys,math,json
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import create_store_kit as k
import bpy
from mathutils import Matrix
k.BASE=k.ROOT/'AssetInbox/Environment/Stores'
k.PREVIEWS=k.ROOT/'Docs/Images/Stores/Cabinets'
SPECS=[
('open_chiller_2500','Açık dikey sütlük','cooler',2.50,.88,2.10,'open',[.38,.72,1.06,1.40,1.74],'dikey-reyonlar'),
('low_chiller_2500','Alçak dikey soğutucu','cooler',2.50,.88,1.50,'open',[.34,.68,1.02],'alcak-dikey-reyonlar'),
('glass_chiller_2500','Cam kapılı sütlük','cooler',2.50,.88,2.10,'doors',[.38,.72,1.06,1.40,1.74],'icten-motorlu-sutlukler'),
('upright_freezer_2100','Üç kapılı dikey dondurucu','freezer',2.10,.85,2.10,'doors',[.38,.72,1.06,1.40,1.74],'dikey-dondurucular'),
('drink_cooler_700','Tek kapılı içecek dolabı','cooler',.70,.72,2.04,'doors',[.38,.72,1.06,1.40,1.74],'tek-kapakli-icecek-dolaplari'),
('drink_cooler_1400','İki kapılı içecek dolabı','cooler',1.40,.72,2.04,'doors',[.38,.72,1.06,1.40,1.74],'2-3-kapakli-icecek-dolaplari'),
('butcher_display_2500','Kasap servis tezgâhı','butcher',2.50,1.16,1.22,'service',[.68],'servis-reyonlari'),
('deli_display_2500','Şarküteri servis tezgâhı','deli',2.50,1.16,1.22,'service',[.68],'servis-reyonlari'),
('island_freezer_2500','Cam sürgülü havuz dondurucu','freezer',2.50,1.05,.94,'island',[.42],'havuz-tipi-dondurucular'),
('cake_display_1500','Pasta teşhir vitrini','service',1.50,.85,1.35,'cake',[.57,.86,1.15],'pasta-ozel-teshir-reyonlari'),
('icecream_display_1500','Dondurma servis vitrini','freezer',1.50,1.00,1.25,'icecream',[],'dondurma-reyonlari'),
('chest_freezer_1500','Kutu tipi dondurucu','freezer',1.50,.78,.90,'island',[.36],'kutu-tipi-sogutucular'),
('tech_table_1800','Teknoloji deneme masası','electronics',1.80,.90,.92,'tech',[],None),
('tech_wall_2400','Teknoloji duvar teşhiri','electronics',2.40,.55,2.00,'techwall',[],None),
('butcher_workbench_1800','Kasap hazırlık tezgâhı','service',1.80,.70,.90,'work',[],None),
]

def preview(path,model,camera,target):
    k.cube('StudioFloor',(0,0,-.035),(30,30,.06),k.mat('MI_StudioFloor',(.36,.38,.40),0,.6),0,False)
    bpy.ops.object.camera_add(location=camera);c=bpy.context.object;c.data.lens=54;k.look_at(c,target);bpy.context.scene.camera=c
    for loc,energy,size in [((1,-3,4),1300,4),((-3,-1,2.7),900,3),((1,3,4),1600,3)]:
        bpy.ops.object.light_add(type='AREA',location=loc);l=bpy.context.object;l.data.energy=energy;l.data.size=size;k.look_at(l,target)
    s=bpy.context.scene;s.render.engine='CYCLES';s.cycles.samples=32;s.cycles.use_denoising=True
    s.render.resolution_x=s.render.resolution_y=1024;s.render.resolution_percentage=100
    s.world.color=(.18,.18,.18);s.view_settings.look='AgX - Medium High Contrast';s.render.filepath=str(path/'preview.png');bpy.ops.render.render(write_still=True)
k.preview=preview

for eid,label,f,w,d,h,style,levels,ref in SPECS:
    k.reset();bpy.context.scene.unit_settings.system='METRIC';bpy.context.scene.unit_settings.scale_length=1
    steel=k.mat('MI_Cabinet_BrushedSteel',(.47,.50,.53),.86,.27)
    dark=k.mat('MI_Cabinet_Anthracite',(.026,.034,.041),.35,.34)
    white=k.mat('MI_Cabinet_Porcelain',(.77,.80,.80),.08,.33)
    black=k.mat('MI_Cabinet_Gasket',(.012,.015,.018),0,.65)
    glass=k.mat('MI_Cabinet_Glass',(.86,.96,.98),0,.08,1)
    glass.node_tree.nodes.get('Principled BSDF').inputs['Alpha'].default_value=1
    led=k.mat('MI_Cabinet_LED',(.85,.95,1),0,.3,emission=(.85,.95,1))
    wood=k.mat('MI_Cabinet_Oak',(.32,.21,.11),0,.44)
    screen=k.mat('MI_Tech_Screen',(.018,.044,.065),.15,.20)
    parts=[]
    def box(name,at,size,mat=dark,bevel=.003,rotation=(0,0,0)):
        o=k.cube(name,at,size,mat,bevel,rotation=rotation);parts.append(o);return o
    def frame(x,y,z,ww,hh):
        for xx in (-ww/2,ww/2):box('GlazingFrame',(x+xx,y,z),(.024,.035,hh),steel,.002)
        for zz in (-hh/2,hh/2):box('GlazingFrame',(x,y,z+zz),(ww,.035,.024),steel,.002)
    def grill(x,y,z,ww,hh):
        box('VentRecess',(x,y,z),(ww,.012,hh),black,0)
        for i in range(max(3,int(hh/.025))):box('Louver',(x,y-.009,z-hh/2+.015+i*.025),(ww,.018,.008),steel,.001)
    for x in (-w/2+.075,w/2-.075):
        for y in (-d/2+.075,d/2-.075):box('AdjustableFoot',(x,y,.035),(.08,.08,.07),black)
    if style in ('open','doors'):
        box('CompressorBase',(0,0,.19),(w,d-.04,.24));box('InsulatedBack',(0,d/2-.045,h/2),(w,.09,h-.08),white)
        for x in (-w/2+.025,w/2-.025):box('EndPanel',(x,0,h/2),(.05,d,h),dark,.008)
        box('Canopy',(0,0,h-.06),(w,d,.12),dark,.008);box('TopLight',(0,-d/2+.10,h-.14),(w-.12,.028,.025),led)
        grill(0,-d/2+.018,.22,w-.20,.15)
        for z in levels:
            box('Shelf',(0,-.01,z-.012),(w-.12,d-.16,.024),white)
            box('PriceChannel',(0,-d/2+.055,z-.018),(w-.12,.024,.038),steel)
            for i in range(max(1,int(w/.4))):box('BlankTicket',(-w/2+.25+i*.4,-d/2+.039,z-.018),(.09,.002,.024),white,.0005)
        for x in (-w/2+.06,w/2-.06):box('VerticalLED',(x,-d/2+.12,h/2+.04),(.018,.025,h-.45),led)
        if style=='doors':
            n=max(1,round(w/.73));dw=(w-.10)/n
            for i in range(n):
                x=-w/2+.05+(i+.5)*dw;zz=(h+.30)/2;hh=h-.38
                box('DoorGlass',(x,-d/2+.055,zz),(dw-.032,.006,hh-.025),glass,.001)
                frame(x,-d/2+.035,zz,dw-.01,hh)
                box('DoorPull',(x+dw/2-.065,-d/2+.01,1.05),(.018,.035,.38),steel)
                for z in (.40,h-.17):box('Hinge',(x-dw/2+.02,-d/2+.02,z),(.035,.05,.07),steel)
        else:
            for x in (-w/2+.026,w/2-.026):box('SideGlass',(x,-.035,h/2+.11),(.006,d-.20,h-.39),glass,.001)
    elif style in ('service','cake','icecream'):
        base=.53 if style!='cake' else .44
        box('LowerBody',(0,.02,base/2+.06),(w,d-.05,base),wood if f=='deli' else white,.016)
        box('KickPlate',(0,-d/2+.018,.13),(w-.06,.025,.13),steel)
        grill(0,-d/2+.006,.27,w-.22,.13)
        bed=.68 if style=='service' else .55
        box('DisplayBed',(0,-.06,bed-.025),(w-.10,d-.23,.05),steel)
        # Sloping front glass and vertical ends leave the operator side open.
        rise=h-bed;angle=math.radians(13)
        box('FrontGlass',(0,-d/2+.13,bed+rise/2),(w-.07,.008,rise),glass,.001,(-angle,0,0))
        box('GlassTop',(0,-.03,h-.01),(w-.06,d-.20,.008),glass,.001)
        for x in (-w/2+.025,w/2-.025):
            box('EndGlass',(x,-.04,bed+rise/2),(.008,d-.21,rise),glass,.001)
            box('GlassSupport',(x,d/2-.16,(h+bed)/2),(.028,.028,h-bed),steel)
        box('TopRail',(0,-d/2+.08,h),(w,.025,.025),steel)
        box('Worktop',(0,d/2-.07,bed+.02),(w,.22,.035),steel)
        if style=='cake':
            for z in levels[1:]:box('GlassShelf',(0,0,z),(w-.11,d-.24,.009),glass,.001);box('ShelfLED',(0,.21,z-.02),(w-.12,.015,.02),led)
        elif style=='icecream':
            for i in range(8):
                x=-w/2+.19+(i%4)*.36;y=-.26+(i//4)*.35
                box('GastronormLip',(x,y,.665),(.31,.30,.015),steel)
                box('EmptyPan',(x,y,.66),(.28,.27,.008),dark)
        else:
            for i in range(5):box('TrayDivider',(-w/2+.20+i*(w-.4)/4,-.06,bed+.035),(.014,d-.28,.055),white)
        box('TemperatureDisplay',(w/2-.18,-d/2+.004,.39),(.12,.012,.05),black)
    elif style=='island':
        box('InsulatedTub',(0,0,.28),(w,d,.44),white,.018)
        box('TubFloor',(0,0,.43),(w-.13,d-.13,.025),steel)
        for x in (-w/2+.025,w/2-.025):box('EndGlass',(x,0,.64),(.006,d-.07,.39),glass,.001)
        for y in (-d/2+.025,d/2-.025):box('SideGlass',(0,y,.64),(w-.06,.006,.39),glass,.001)
        for i in range(2):
            y=(i-.5)*(d-.06)/2;z=h-.025+i*.014
            box('SlidingLid',(0,y,z),(w-.08,(d-.08)/2,.008),glass,.001)
            for yy in (-1,1):box('LidRail',(0,y+yy*(d-.08)/4,z),(w-.06,.024,.018),steel)
            box('LidHandle',(.15,y,z+.013),(.18,.027,.022),dark)
        for i in range(6):box('BasketDivider',(-w/2+.18+i*(w-.36)/5,0,.59),(.008,d-.14,.30),white,.001)
        grill(w/2-.24,-d/2-.002,.27,.34,.16)
    elif style in ('tech','work'):
        for x in (-w/2+.08,w/2-.08):box('Leg',(x,0,.43),(.08,d-.12,.86),steel)
        box('Tabletop',(0,0,h-.03),(w,d,.06),wood if style=='tech' else steel,.008)
        box('UnderShelf',(0,0,.22),(w-.16,d-.12,.035),steel)
        if style=='tech':
            for i in range(4):
                x=(i-1.5)*.39;box('SecurityDock',(x,0,h+.015),(.15,.15,.03),white)
                box('Tablet',(x,.01,h+.16),(.24,.022,.19),dark,.008,(math.radians(20),0,0))
                box('TabletScreen',(x,-.005,h+.16),(.215,.004,.165),screen,.001,(math.radians(20),0,0))
        else:box('CuttingBoard',(0,-.04,h+.012),(.62,.48,.024),white)
    else:
        box('Back',(0,d/2-.025,h/2),(w,.05,h),wood)
        for z in (.32,.91,1.55):box('Shelf',(0,0,z),(w,d,.035),steel)
        for i in range(3):
            x=(i-1)*.70;box('TV',(x,.10,1.24),(.62,.04,.40),dark,.008);box('TVScreen',(x,.076,1.24),(.58,.004,.36),screen,.001)
    name='SM_'+''.join(word.title() for word in eid.split('_'))
    model=k.join(parts,name,eid,(w*1000,d*1000,h*1000));bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
    cols=[k.collision(name,0,(0,0,.22),(w,d,.44))]
    if style in ('open','doors','techwall'):cols.append(k.collision(name,1,(0,d/2-.04,h/2),(w,.08,h)))
    if style in ('service','cake','icecream'):cols.append(k.collision(name,1,(0,-d/2+.06,h/2),(w,.07,h)))
    # Include handles/devices in physical envelope; bottom remains on the floor.
    verts=[model.matrix_world@v.co for v in model.data.vertices];dims=[(max(v[i] for v in verts)-min(v[i] for v in verts))*1000 for i in range(3)]
    zones=[dict(id=f'front_l{i:02d}',face='front',level=i,centerCm=[0,0,z*100],usableWidthCm=w*100-12,usableDepthCm=d*100-20,clearanceHeightCm=(levels[i+1]-z)*100-4 if i+1<len(levels) else max(15,(h-z)*100-6)) for i,z in enumerate(levels)]
    meta=dict(schemaVersion=1,id=eid,displayName=label,mesh=name+'.fbx',unrealMesh=f'/Game/Stores/Equipment/{name}.{name}',family=f,dimensionsMm=dict(zip(['width','depth','height'],[round(v,2) for v in dims])),origin='floor_center',frontAxis='-Y',materials=[m.name for m in model.data.materials],collision=dict(policy='custom_ucx',pieces=len(cols)),zones=zones,checkouts=0,planogram=dict(doubleSided=False,widthCm=w*100-12,depthCm=d*100-20,frontY=-d*50+10,meshYaw=180),reference=dict(url='http://www.buzrefrigeration.com/tr/urunler/'+ref if ref else '',accuracy='Reference-inspired game model; dimensions are authored, not manufacturer specifications'),lod=dict(naniteCandidate=False,lod1TriangleRatio=.5,lod2TriangleRatio=.25))
    if style in ('open','doors','service','island','cake','icecream'):
        meta['planogram']['showCategorySign'] = False
    k.save_asset(eid,name,model,cols,meta,(w*1.25+1,-d*3.3-1,h*1.1+1),(0,0,h*.52))
    # Correct export without saving the temporary centimetre geometry into the source.
    bpy.ops.object.select_all(action='DESELECT')
    for o in [model,*cols]:
        o.data.transform(o.matrix_world);o.matrix_world=Matrix.Identity(4);o.data.transform(Matrix.Scale(100,4));o.select_set(True)
    bpy.context.view_layer.objects.active=model;bpy.context.scene.unit_settings.scale_length=.01
    bpy.ops.export_scene.fbx(filepath=str(k.BASE/eid/(name+'.fbx')),use_selection=True,object_types={'MESH'},apply_unit_scale=True,apply_scale_options='FBX_SCALE_UNITS',axis_forward='-Y',axis_up='Z',mesh_smooth_type='FACE',add_leaf_bones=False,bake_anim=False)
print('MIRAS_CABINET_LIBRARY_READY')
