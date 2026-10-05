"""Render source HQ GLB before Unreal to distinguish source and import defects."""
import bpy, sys
from pathlib import Path
from mathutils import Vector
args = sys.argv[sys.argv.index('--') + 1:]
source, dest = map(Path, args)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(source))
for material in bpy.data.materials:
    texture = next((n for n in material.node_tree.nodes if n.type == 'TEX_IMAGE'), None)
    output = next(n for n in material.node_tree.nodes if n.type == 'OUTPUT_MATERIAL')
    emission = material.node_tree.nodes.new('ShaderNodeEmission')
    if texture:
        material.node_tree.links.new(texture.outputs['Color'], emission.inputs['Color'])
    material.node_tree.links.new(emission.outputs[0], output.inputs['Surface'])
bpy.ops.object.camera_add(location=(0, 0, .24585))
camera = bpy.context.object
camera.rotation_euler = Vector((0, 1, 0)).to_track_quat('-Z', 'Y').to_euler()
camera.data.lens = 23
scene = bpy.context.scene
scene.camera = camera
scene.render.engine = 'CYCLES'
scene.cycles.samples = 8
scene.render.resolution_x = 1280
scene.render.resolution_y = 720
scene.render.resolution_percentage = 100
scene.view_settings.view_transform = 'Standard'
scene.render.filepath = str(dest.resolve())
bpy.ops.render.render(write_still=True)
