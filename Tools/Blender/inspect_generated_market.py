"""Inspect a GLB in Blender without changing its source."""
import bpy, json, sys
from pathlib import Path
from mathutils import Vector
args = sys.argv[sys.argv.index('--') + 1:]
source, output = map(Path, args)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(source))
meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
vertices = [o.matrix_world @ v.co for o in meshes for v in o.data.vertices]
mins = [min(v[i] for v in vertices) for i in range(3)]
maxs = [max(v[i] for v in vertices) for i in range(3)]
data = {'source': str(source), 'bounds_m': [mins, maxs], 'meshes': [], 'images': []}
hit, pos, normal, index, obj, matrix = bpy.context.scene.ray_cast(bpy.context.evaluated_depsgraph_get(), Vector((0, 0, 0)), Vector((0, 0, -1)))
data['floor_at_source_camera_m'] = pos.z if hit else None
for o in meshes:
    data['meshes'].append({'name': o.name, 'vertices': len(o.data.vertices), 'triangles': sum(len(p.vertices)-2 for p in o.data.polygons), 'uv_layers': len(o.data.uv_layers), 'materials': [m.name for m in o.data.materials if m]})
for img in bpy.data.images:
    data['images'].append({'name': img.name, 'size': list(img.size)})
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(data, indent=2), encoding='utf-8')
print('GENERATED_MARKET_INSPECTED', json.dumps(data))
