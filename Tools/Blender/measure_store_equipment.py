"""Update the physical envelope from generated mesh + UCX, and verify Blender's axis/origin contract."""
import sys,json
from pathlib import Path
import bpy
from mathutils import Vector
root=Path(sys.argv[sys.argv.index('--')+1]).resolve()
bpy.context.preferences.filepaths.save_version=0
for metadata in sorted((root/'AssetInbox/Environment/Stores').rglob('equipment.json')):
    data=json.loads(metadata.read_text(encoding='utf-8'))
    if 'planogram' not in data: continue
    source=next((metadata.parent/'Source').glob('*.blend'))
    bpy.ops.wm.open_mainfile(filepath=str(source))
    model=bpy.data.objects[Path(data['mesh']).stem]
    if model.location.length>.00001 or any(abs(v-1)>.00001 for v in model.scale) or model.rotation_euler.to_quaternion().angle>.00001: raise RuntimeError('Transform not applied: '+model.name)
    objects=[model,*[o for o in bpy.data.objects if o.name.startswith('UCX_'+model.name+'_')]]
    if len(objects)-1!=data['collision']['pieces']: raise RuntimeError('UCX count: '+model.name)
    points=[o.matrix_world@Vector(p) for o in objects for p in o.bound_box]
    if min(p.z for p in points)<-.00001: raise RuntimeError('Below floor: '+model.name)
    dims=[round((max(p[i] for p in points)-min(p[i] for p in points))*1000,2) for i in range(3)]
    data['dimensionsMm']=dict(zip(('width','depth','height'),dims))
    model['dimensions_mm']=','.join(str(v) for v in dims)
    metadata.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    bpy.ops.wm.save_as_mainfile(filepath=str(source))
    print('STORE_BLENDER_VALIDATED='+data['id']+': '+str(dims))
