"""Authored geometry in metres; no generated-world geometry or image atlas.

The legacy FBX importer reflects Y and StoreKit rotates shells 180 degrees.
Author X reversed so the final game coordinates match the floor plan.
"""
import sys, json, math
from pathlib import Path
import bpy
from mathutils import Matrix, Vector
sys.path.insert(0, str(Path(__file__).parent))
from create_store_kit import reset, mat, cube, join, collision, look_at

root = Path(sys.argv[sys.argv.index('--')+1]).resolve()
out = root/'AssetInbox/Environment/Stores/HandmadeNeighborhood'
out.mkdir(parents=True, exist_ok=True)
(out/'Source').mkdir(exist_ok=True)
reset()
bpy.context.scene.unit_settings.system = 'METRIC'
bpy.context.scene.unit_settings.scale_length = 1
materials = {
    'Floor': mat('HM_Floor', (.64,.61,.54), roughness=.42),
    'Wall': mat('HM_Wall', (.78,.75,.67), roughness=.8),
    'Wood': mat('HM_Wood', (.25,.11,.045), roughness=.5),
    'Metal': mat('HM_Metal', (.025,.045,.04), metallic=.65, roughness=.35),
    'Green': mat('HM_Green', (.035,.14,.095), roughness=.52),
    'Concrete': mat('HM_Concrete', (.38,.40,.39), roughness=.85),
    'Road': mat('HM_Road', (.07,.085,.09), roughness=.95),
    'Glass': mat('HM_Glass', (.60,.75,.72), roughness=.06, transmission=1),
    'Light': mat('HM_Light', (.9,.85,.66), emission=(1,.85,.65)),
}
parts=[]; hulls=[]
name='SM_HandmadeNeighborhood'
def box(label, p, size, surface='Wall', solid=False, bevel=.008):
    p=(-p[0],p[1],p[2])
    o=cube(label,p,size,materials[surface],bevel)
    parts.append(o)
    if solid: hulls.append(collision(name,len(hulls),p,size))
    return o

# 8 x 10 metres, zero-step 1.8m entry at front centre. Every collision
# primitive corresponds to a solid part; never bridge the opening with a hull.
box('StoreSlab',(0,0,-.10),(8.2,10.2,.20),'Floor',True)
box('LeftWall',(-4.05,.65,1.55),(.18,8.7,3.1),'Wall',True)
box('RightWall',(4.05,0,1.55),(.18,10.2,3.1),'Wall',True)
box('BackWall',(0,5.05,1.55),(8.2,.18,3.1),'Wall',True)
box('Roof',(0,0,3.19),(8.3,10.3,.18),'Wall',True)
for x in (-2.5,2.5):
    box('FrontPlinth',(x,-5.03,.12),(3.0,.14,.24),'Concrete',True)
    box('FrontGlass',(x,-5.03,1.55),(2.96,.025,2.62),'Glass',True,0)
for x in (-3.99,-2.5,-.94,.94,2.5,3.99):
    box('FrontMullion',(x,-5.04,1.5),(.055,.09,3),'Metal',True)
box('FrontHeader',(0,-5.04,2.97),(8.1,.12,.20),'Metal',True)
box('EntryLintel',(0,-5.04,2.73),(1.82,.13,.13),'Metal',True)
box('EntryTransom',(0,-5.04,2.88),(1.80,.025,.19),'Glass',False,0)
# Glazed corner wraps around the left side, opening the view toward the pavement.
box('CornerPlinth',(-4.05,-4.34,.12),(.18,1.4,.24),'Concrete',True)
box('CornerGlass',(-4.05,-4.34,1.55),(.025,1.38,2.62),'Glass',True,0)
for y in (-5,-3.65): box('CornerPost',(-4.05,y,1.5),(.09,.055,3),'Metal',True)
box('CornerHeader',(-4.05,-4.34,2.97),(.12,1.4,.20),'Metal',True)
# Fixed open glass leaf alongside entry, no collision across the walking path.
box('OpenDoorLeaf',(.97,-4.58,1.33),(.025,.82,2.60),'Glass',True,0)
for y in (-4.99,-4.17): box('DoorEdge',(.97,y,1.33),(.04,.04,2.64),'Metal')
box('DoorHandle',(.90,-4.27,1.20),(.025,.035,.40),'Metal')
box('Awning',(0,-5.55,3.09),(8.5,1.30,.13),'Green')
box('Fascia',(0,-6.15,3.05),(8.5,.09,.34),'Green')
box('SignBoard',(0,-6.20,3.30),(6,.09,.52),'Green')
bpy.ops.object.text_add(location=(0,-6.26,3.16),rotation=(math.pi/2,0,0))
t=bpy.context.object; t.name='MarketSign'; t.data.body='MAHALLE MARKET'; t.data.align_x='CENTER'; t.data.size=.31; t.data.extrude=.003
t.data.materials.append(materials['Light'])
bpy.ops.object.convert(target='MESH'); parts.append(bpy.context.object)
for x in (-3.75,3.75): box('FacadePier',(x,-5.12,3.50),(.30,.30,.70),'Wall')
# A compact stock room behind the cooler; real door opening, no hidden wall.
box('StockroomSide',(2.65,4.2,1.55),(.12,1.7,3.1),'Wall',True)
box('StockroomFrontRight',(3.87,3.35,1.55),(.25,.12,3.1),'Wall',True)
box('StockroomFrontLeft',(2.83,3.35,1.55),(.25,.12,3.1),'Wall',True)
box('StockroomLintel',(3.35,3.35,2.71),(.80,.12,.78),'Wall',True)
box('StockroomOpenDoor',(2.95,3.78,1.10),(.055,.80,2.2),'Wood',True)
for x in (-3.92,3.92): box('Skirting',(x,.0,.085),(.04,9.80,.17),'Metal')
box('BackSkirting',(0,4.93,.085),(7.8,.04,.17),'Metal')
for y in (-3.9,-1.8,.3,2.4,4.4):
    box('CeilingBatton',(0,y,3.07),(7.8,.045,.06),'Wood')
for y in (-2.4,.7,3.4):
    box('LightHousing',(0,y,2.97),(2.3,.18,.07),'Metal')
    box('LightDiffuser',(0,y,2.93),(2.21,.12,.025),'Light')
# Pavement flush at the entrance, curb outside the walking area.
box('Pavement',(0,-6.65,-.10),(16,3.25,.20),'Concrete',True)
box('Street',(0,-11,-.24),(28,5.4,.20),'Road',True)
box('Curb',(0,-8.28,-.10),(16,.16,.20),'Concrete',True)
for x in range(-7,8): box('PavingJoint',(x,-6.7,.001),(.012,3,.004),'Metal',False,0)
for y in (-5.5,-6.5,-7.5): box('PavingJoint',(0,y,.001),(16,.012,.004),'Metal',False,0)
for x in (-5.9,5.9):
    box('Planter',(x,-6.0,.32),(1.1,.70,.64),'Green',True,.03)
    for dx in (-.3,0,.3):
        bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2,radius=.32,location=(-(x+dx),-6,.86))
        o=bpy.context.object; o.name='Plant'; o.scale=(1,.75,1); o.data.materials.append(materials['Green']); parts.append(o)
box('EntryMat',(0,-4.65,.007),(1.65,.55,.012),'Metal')
editable=bpy.data.collections.new('EditableArchitecture')
bpy.context.scene.collection.children.link(editable)
export_parts=[]
for o in parts:
    duplicate=o.copy(); duplicate.data=o.data.copy(); bpy.context.scene.collection.objects.link(duplicate); export_parts.append(duplicate)
    for c in list(o.users_collection): c.objects.unlink(o)
    editable.objects.link(o); o.hide_render=True; o.hide_set(True)
model=join(export_parts,name,'handmade_neighborhood',(8000,10000,3100))
# Original named pieces remain in EditableArchitecture; combined mesh is export preview.
model['authorship']='Parametric Blender geometry; no AI world mesh'
meta={'mesh':name+'.fbx','dimensionsCm':[800,1000,310],'entryWidthCm':180,'floorCm':0,'collision':{'pieces':len(hulls)},'materials':list(materials),'source':'Source/HandmadeNeighborhood.blend'}
(out/'equipment.json').write_text(json.dumps(meta,indent=2),encoding='utf-8')
# A source preview camera facing into the entrance; lights stay editable.
bpy.ops.object.camera_add(location=(6,-12,5.4)); camera=bpy.context.object
look_at(camera,(0,-1,1.2)); camera.data.lens=38; bpy.context.scene.camera=camera
for p,energy,size in (((0,-2,2.8),1500,5),((0,-7,6),2200,8)):
    bpy.ops.object.light_add(type='AREA',location=p); light=bpy.context.object; light.data.energy=energy; light.data.shape='DISK'; light.data.size=size; look_at(light,(0,0,0))
bpy.context.scene.world.color=(.3,.35,.4)
bpy.context.scene.render.engine='BLENDER_EEVEE'
bpy.context.scene.render.resolution_x=1280; bpy.context.scene.render.resolution_y=720; bpy.context.scene.render.resolution_percentage=100
bpy.context.scene.render.image_settings.file_format='PNG'; bpy.context.scene.render.filepath=str(out/'preview.png')
bpy.ops.wm.save_as_mainfile(filepath=str(out/'Source/HandmadeNeighborhood.blend'))
bpy.ops.render.render(write_still=True)
# Centimetre export copy; matching baked render and UCX transforms.
bpy.ops.object.select_all(action='DESELECT')
for o in [model]+hulls:
    o.hide_set(False); o.data.transform(o.matrix_world); o.matrix_world=Matrix.Identity(4); o.data.transform(Matrix.Scale(100,4)); o.select_set(True)
bpy.context.view_layer.objects.active=model; bpy.context.scene.unit_settings.scale_length=.01
bpy.ops.export_scene.fbx(filepath=str(out/(name+'.fbx')),use_selection=True,object_types={'MESH'},apply_unit_scale=True,apply_scale_options='FBX_SCALE_UNITS',axis_forward='-Y',axis_up='Z',use_mesh_modifiers=True,mesh_smooth_type='FACE',add_leaf_bones=False,bake_anim=False)
print('HANDMADE_STORE_CREATED hulls='+str(len(hulls)))
