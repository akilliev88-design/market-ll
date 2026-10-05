"""Three independent Blender buildings, matching the existing store size contracts."""
import sys,json,math
from pathlib import Path
import bpy
from mathutils import Matrix
sys.path.insert(0,str(Path(__file__).parent))
from create_store_kit import reset,mat,cube,join,collision
root=Path(sys.argv[sys.argv.index('--')+1]).resolve()
for store in json.loads((root/'Config/magazalar.json').read_text(encoding='utf-8'))['stores']:
    if store['format']=='mahalle':continue
    sid=store['id'];w,d=[x/100 for x in store['footprintCm']];h=store['ceilingCm']/100
    folder=root/'AssetInbox/Environment/Stores/HandmadeLarge'/sid;folder.mkdir(parents=True,exist_ok=True);(folder/'Source').mkdir(exist_ok=True)
    reset();bpy.context.scene.unit_settings.system='METRIC';bpy.context.scene.unit_settings.scale_length=1
    surfaces={'Floor':mat('HL_Floor',(.64,.63,.59),roughness=.45),'Wall':mat('HL_Wall',(.78,.78,.74),roughness=.82),'Metal':mat('HL_Metal',(.03,.05,.055),metallic=.5),'Glass':mat('HL_Glass',(.7,.8,.8),roughness=.06,transmission=1),'Accent':mat('HL_Accent',(.025,.14,.12)),'Concrete':mat('HL_Concrete',(.45,.46,.46),roughness=.8),'Light':mat('HL_Light',(.8,.85,.9),emission=(1,1,1))}
    parts=[];hulls=[];name='SM_Handmade_'+sid
    def box(label,p,size,surface='Wall',solid=False):
        p=(-p[0],p[1],p[2]);o=cube(label,p,size,surfaces[surface],.006);parts.append(o)
        if solid:hulls.append(collision(name,len(hulls),p,size))
    box('Floor',(0,0,-.12),(w+.24,d+.24,.24),'Floor',True)
    for x in (-w/2-.1,w/2+.1):box('SideWall',(x,0,h/2),(.20,d+.4,h),'Wall',True)
    roof=cube('SM_HandmadeRoof_'+sid,(0,0,h+.10),(w+.45,d+.45,.20),surfaces['Wall'],.006)
    bpy.context.scene.cursor.location=(0,0,0);bpy.context.view_layer.objects.active=roof;bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    entry=store['points']['entrance']['at'][0]/100;opening={'kucuk':2.4,'buyuk':4,'hiper':6}[store['format']]
    gh=min(h,3.6)
    segments=[(-w/2,entry-opening/2),(entry+opening/2,w/2)]
    for a,b in segments:
        n=max(1,math.ceil((b-a)/2.5));step=(b-a)/n
        box('WindowPlinth',((a+b)/2,-d/2-.05,.10),(b-a,.15,.20),'Concrete',True)
        for i in range(n):
            x=a+(i+.5)*step
            box('Window',(x,-d/2-.06,(gh+.20)/2),(step-.06,.025,gh-.20),'Glass',True)
        for i in range(n+1):box('Mullion',(a+i*step,-d/2-.09,gh/2),(.055,.10,gh),'Metal',True)
    box('FrontHeader',(0,-d/2-.1,gh+.05),(w+.25,.20,.15),'Metal',True)
    if h>gh+.1:box('FrontUpperWall',(0,-d/2-.05,(gh+h)/2),(w+.3,.20,h-gh),'Wall',True)
    box('Canopy',(entry,-d/2-.7,gh),(max(opening+3,w*.28),1.5,.15),'Accent')
    box('Fascia',(entry,-d/2-1.43,gh),(max(opening+3,w*.28),.10,.6),'Accent')
    bpy.ops.object.text_add(location=(-entry,-d/2-1.50,gh-.13),rotation=(math.pi/2,0,0))
    text=bpy.context.object;text.data.body={'kucuk':'MARKET','buyuk':'SUPERMARKET','hiper':'HIPERMARKET'}[store['format']];text.data.align_x='CENTER';text.data.size=.34;text.data.extrude=.003;text.data.materials.append(surfaces['Light']);bpy.ops.object.convert(target='MESH');parts.append(bpy.context.object)
    back=store['points']['backroom']['min'][1]/100
    def wall_with_door(y,door,width):
        for a,b in [(-w/2,door-width/2),(door+width/2,w/2)]:box('RearWall',((a+b)/2,y,h/2),(b-a,.16,h),'Wall',True)
        box('RearLintel',(door,y,(h+2.6)/2),(width,.16,h-2.6),'Wall',True)
    depot=store['points'].get('depotDoor',{'at':[w/2-3,back*100,0]})['at'][0]/100
    wall_with_door(back,depot,2.0)
    receiving=store['points']['receiving']['at'][0]/100
    wall_with_door(d/2,receiving,2.4)
    box('Pavement',(0,-d/2-2,-.10),(w+7,4,.20),'Concrete',True)
    for ob in store.get('architecture',{}).get('obstacles',[]):
        p=[v/100 for v in ob['at']];sz=[v/100 for v in ob['sizeCm']];p[2]+=sz[2]/2;box(ob['id'],p,sz,'Wall',True)
    editable=bpy.data.collections.new('EditableArchitecture');bpy.context.scene.collection.children.link(editable);export=[]
    for o in parts:
        du=o.copy();du.data=o.data.copy();bpy.context.scene.collection.objects.link(du);export.append(du)
        for collection in list(o.users_collection):collection.objects.unlink(o)
        editable.objects.link(o);o.hide_set(True);o.hide_render=True
    model=join(export,name,sid,(w*1000,d*1000,h*1000))
    bpy.ops.wm.save_as_mainfile(filepath=str(folder/'Source'/(sid+'.blend')))
    bpy.ops.object.select_all(action='DESELECT')
    for o in [model]+hulls:o.hide_set(False);o.data.transform(o.matrix_world);o.matrix_world=Matrix.Identity(4);o.data.transform(Matrix.Scale(100,4));o.select_set(True)
    bpy.context.view_layer.objects.active=model;bpy.context.scene.unit_settings.scale_length=.01
    bpy.ops.export_scene.fbx(filepath=str(folder/(name+'.fbx')),use_selection=True,object_types={'MESH'},apply_unit_scale=True,apply_scale_options='FBX_SCALE_UNITS',axis_forward='-Y',axis_up='Z',use_mesh_modifiers=True,mesh_smooth_type='FACE',add_leaf_bones=False,bake_anim=False)
    bpy.ops.object.select_all(action='DESELECT');roof.data.transform(roof.matrix_world);roof.matrix_world=Matrix.Identity(4);roof.data.transform(Matrix.Scale(100,4));roof.select_set(True);bpy.context.view_layer.objects.active=roof
    bpy.ops.export_scene.fbx(filepath=str(folder/(roof.name+'.fbx')),use_selection=True,object_types={'MESH'},apply_unit_scale=True,apply_scale_options='FBX_SCALE_UNITS',axis_forward='-Y',axis_up='Z',use_mesh_modifiers=True,mesh_smooth_type='FACE',add_leaf_bones=False,bake_anim=False)
    meta=dict(mesh=name+'.fbx',id=sid,dimensionsCm=[w*100,d*100,h*100],entryWidthCm=opening*100,collision={'pieces':len(hulls)},unrealMesh=f'/Game/Stores/Handmade/Large/{sid}/{name}.{name}')
    (folder/'equipment.json').write_text(json.dumps(meta,indent=2))
    print('LARGE_BUILDING_CREATED='+sid,flush=True)
