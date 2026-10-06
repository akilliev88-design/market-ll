"""Validate authored source geometry and furniture-only exports independently of rendering."""
import json
import sys
from pathlib import Path
import bpy
root = Path(sys.argv[sys.argv.index('--') + 1]).resolve()
reports = []
for eid in ('tv_wall_4800', 'tv_plinth_2400', 'tv_island_3000'):
    folder = root / 'AssetInbox/Environment/Stores' / eid
    meta = json.loads((folder / 'equipment.json').read_text())
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'Source' / (eid + '.blend')))
    model = bpy.data.objects[Path(meta['mesh']).stem]
    assert model.type == 'MESH'
    assert model.location.length < .0001 and all(abs(v - 1) < .001 for v in model.scale)
    assert model.dimensions.x < 5 and model.dimensions.z < 3
    assert len([o for o in bpy.data.objects if o.name.startswith('UCX_')]) == meta['collision']['pieces']
    assert any(o.name.startswith('PreviewOnly_TV') for o in bpy.data.objects)
    # Read the actual export, not the staged .blend, to catch accidental baked-in TV devices.
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(folder / meta['mesh']))
    meshes = [o for o in bpy.data.objects if o.type == 'MESH']
    assert len(meshes) == 1 + meta['collision']['pieces'], [o.name for o in meshes]
    assert not any('PreviewOnly' in o.name for o in meshes)
    reports.append(dict(id=eid, source='metres', export='furniture_only', collisionPieces=meta['collision']['pieces']))
for inches in (32, 43, 55, 65, 75):
    folder = root / 'AssetInbox/Products/Televisions' / f'tv_{inches}'
    meta = json.loads((folder / 'product.json').read_text())
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'Source' / f'tv_{inches}.blend'))
    model = bpy.data.objects[f'SM_Television{inches}']
    assert model.location.length < .0001 and all(abs(v - 1) < .001 for v in model.scale)
    for actual, expected in zip(model.dimensions, (meta['depthMm'], meta['widthMm'], meta['heightMm'])):
        assert abs(actual * 1000 - expected) < 2, (inches, actual, expected)
    assert model.data.uv_layers.active
    image = next(n.image for m in model.data.materials for n in m.node_tree.nodes if n.type == 'TEX_IMAGE')
    assert image.packed_file
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(folder / meta['mesh']))
    assert len([o for o in bpy.data.objects if o.type == 'MESH']) == 1
    reports.append(dict(id=meta['id'], source='metres', export='independent_product', widthMm=meta['widthMm']))
(root / 'Saved/Logs/TV_assets.json').write_text(json.dumps(reports, indent=2))
print('SIM_TV_ASSETS_VALIDATED=8/8')
