"""Verify authored sources, origins, units, collision, real envelopes and zone clearance."""
import sys,json
from pathlib import Path
import bpy
root=Path(sys.argv[sys.argv.index('--')+1]);count=0
for p in sorted((root/'AssetInbox/Environment/Stores').rglob('equipment.json')):
    d=json.loads(p.read_text(encoding='utf-8'))
    if 'reference' not in d:continue
    bpy.ops.wm.open_mainfile(filepath=str(next((p.parent/'Source').glob('*.blend'))))
    assert bpy.context.scene.unit_settings.scale_length==1,d['id']
    mesh=bpy.data.objects[Path(d['mesh']).stem];assert mesh.location.length<.001,d['id'];assert all(abs(v-1)<.0001 for v in mesh.scale)
    verts=[mesh.matrix_world@v.co for v in mesh.data.vertices]
    assert abs(min(v.z for v in verts))<.002,d['id']
    for i,key in enumerate(('width','depth','height')):
        measured=(max(v[i] for v in verts)-min(v[i] for v in verts))*1000
        assert abs(measured-d['dimensionsMm'][key])<.02,(d['id'],key,measured)
    cols=[o for o in bpy.data.objects if o.name.startswith('UCX_'+mesh.name+'_')];assert len(cols)==d['collision']['pieces']
    triangles=sum(len(poly.vertices)-2 for poly in mesh.data.polygons)
    assert triangles<30000,(d['id'],triangles)
    for z in d['zones']:assert z['clearanceHeightCm']>0
    print(f'CABINET_CHECK {d["id"]}: {triangles} triangles, {len(cols)} UCX, origin=0, metre source',flush=True);count+=1
assert count==15,count
print('CABINET_LIBRARY_VALIDATED=15')
