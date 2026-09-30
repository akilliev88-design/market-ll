"""Bake a centimetre export copy so render and UCX vertices have identical units.

The .blend sources stay in metres, unit scale 1.0. No source is saved by this script.
"""
import sys,json
from pathlib import Path
import bpy
from mathutils import Matrix

def export_all(root):
    for metadata in sorted((root/'AssetInbox/Environment/Stores').rglob('equipment.json')):
        data=json.loads(metadata.read_text(encoding='utf-8'))
        source=next((metadata.parent/'Source').glob('*.blend'))
        bpy.ops.wm.open_mainfile(filepath=str(source))
        name=Path(data['mesh']).stem
        objects=[o for o in bpy.data.objects if o.name==name or o.name.startswith('UCX_'+name+'_')]
        bpy.ops.object.select_all(action='DESELECT')
        for o in objects:
            o.hide_set(False)
            o.data.transform(o.matrix_world)
            o.matrix_world=Matrix.Identity(4)
            o.data.transform(Matrix.Scale(100,4))
            o.select_set(True)
        bpy.context.view_layer.objects.active=bpy.data.objects[name]
        bpy.context.scene.unit_settings.scale_length=.01
        bpy.ops.export_scene.fbx(filepath=str(metadata.parent/data['mesh']),use_selection=True,object_types={'MESH'},
            apply_unit_scale=True,apply_scale_options='FBX_SCALE_UNITS',axis_forward='-Y',axis_up='Z',
            use_mesh_modifiers=True,mesh_smooth_type='FACE',add_leaf_bones=False,bake_anim=False)
        print('STORE_FBX_CENTIMETRES='+name)
if __name__=='__main__': export_all(Path(sys.argv[sys.argv.index('--')+1]).resolve())
