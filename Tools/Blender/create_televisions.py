"""Five independent televisions; product axes +X front, +Y width, +Z up, floor pivot."""
import json
import math
import sys
from pathlib import Path
import bpy
from mathutils import Matrix
sys.path.insert(0, str(Path(__file__).resolve().parent))
import create_store_kit as k

BASE = k.ROOT / 'AssetInbox/Products/Televisions'
BASE.mkdir(parents=True, exist_ok=True)


def picture():
    image = bpy.data.images.new('T_TV_DemoLandscape', width=512, height=288)
    pixels = []
    for y in range(288):
        v = y / 287
        for x in range(512):
            u = x / 511
            ridge = .49 + .09 * math.sin(u * 13) + .035 * math.sin(u * 37)
            foreground = .19 + .07 * math.sin(u * 9 + 2)
            if v > ridge:
                c = (.055 + .2 * (1 - v), .34 + .28 * (1 - v), .83)
                if (u - .77)**2 + (v - .78)**2 < .003: c = (1, .85, .30)
            elif v > foreground:
                if u < .56 + .07 * math.sin(v * 25):
                    c = (.03, .28 + .13 * math.sin(v * 100)**2, .70)
                else: c = (.045, .28 + .32 * v, .13)
            else:
                c = (.14 + .07 * math.sin(u * 120)**2, .42 + .1 * v, .08)
            pixels.extend((*c, 1))
    image.pixels.foreach_set(pixels)
    image.filepath_raw = str(BASE / 'T_TV_DemoLandscape.png')
    image.file_format = 'PNG'
    image.save()
    return image


profiles = []
for inches in (32, 43, 55, 65, 75):
    k.reset()
    bpy.context.scene.unit_settings.system = 'METRIC'
    bpy.context.scene.unit_settings.scale_length = 1
    diagonal = inches * .0254
    sw = diagonal * 16 / math.sqrt(337)
    sh = diagonal * 9 / math.sqrt(337)
    width, height, depth = sw + .022, sh + .084, .24 + inches * .0006
    eid = f'tv_{inches}'
    name = f'SM_Television{inches}'
    folder = BASE / eid
    (folder / 'Source').mkdir(parents=True, exist_ok=True)
    dark = k.mat('MI_TV_ProductBezel', (.009, .012, .016), .3, .27)
    back = k.mat('MI_TV_ProductBack', (.026, .029, .033), .05, .55)
    steel = k.mat('MI_TV_ProductFoot', (.075, .08, .09), .8, .23)
    glass = k.mat('MI_TV_ProductScreen', (.04, .08, .13), 0, .14)
    shader = glass.node_tree.nodes.get('Principled BSDF')
    image = picture()
    texture = glass.node_tree.nodes.new('ShaderNodeTexImage')
    texture.image = image
    glass.node_tree.links.new(texture.outputs['Color'], shader.inputs['Base Color'])
    glass.node_tree.links.new(texture.outputs['Color'], shader.inputs['Emission Color'])
    shader.inputs['Emission Strength'].default_value = .8
    parts = []
    def box(label, at, size, material, bevel=.003):
        o = k.cube(label, at, size, material, bevel)
        parts.append(o)
        return o
    center = height - sh / 2 - .011
    box('SlimBezel', (0, 0, center), (.038, width, sh + .022), dark, .005)
    screen = box('GlassScreen', (.020, 0, center), (.002, sw, sh), glass, 0)
    # Front UVs map the landscape without the default cube atlas seams.
    uv = screen.data.uv_layers.active
    for polygon in screen.data.polygons:
        for loop_index in polygon.loop_indices:
            vertex = screen.data.vertices[screen.data.loops[loop_index].vertex_index].co
            uv.data[loop_index].uv = (vertex.y / sw + .5, vertex.z / sh + .5)
    box('RearElectronics', (-.038, 0, center - sh * .19), (.065, width * .72, sh * .43), back, .009)
    for side in (-1, 1):
        y = side * width * .32
        box('StandLeg', (-.008, y, .052), (.025, .023, .104), steel)
        box('StandFoot', (.003, y, .012), (depth, .055, .024), steel)
        for i in range(12):
            box('RearVent', (-.072, side * width * .24 + (i - 5.5) * .009, center - .08),
                (.003, .003, sh * .14), dark, 0)
    box('ConnectionRecess', (-.073, width * .23, center - .14), (.005, .07, .06), dark, 0)
    box('PowerIndicator', (.021, width * .38, height - sh - .022), (.002, .008, .003), steel, 0)
    model = k.join(parts, name, eid, (width * 1000, depth * 1000, height * 1000))
    model['product_axes'] = '+X front / +Y width / +Z up'
    model['diagonal_inches'] = inches
    metadata = dict(id=eid, inches=inches, mesh=name + '.fbx',
                    unrealMesh=f'/Game/Stores/Televisions/{name}.{name}',
                    widthMm=round(width * 1000), depthMm=round(depth * 1000), heightMm=round(height * 1000),
                    origin='bottom_center', frontAxis='+X', materials=[m.name for m in model.data.materials],
                    technology='unspecified', note='Generic authored geometry. Screen diagonal is measured; no branded specifications claimed.')
    (folder / 'product.json').write_text(json.dumps(metadata, indent=2), encoding='utf-8')
    k.preview(folder, model, (width * 1.7 + .4, -width * 1.1, height * 1.15), (0, 0, height / 2))
    image.pack()
    bpy.ops.wm.save_as_mainfile(filepath=str(folder / 'Source' / (eid + '.blend')))
    bpy.ops.object.select_all(action='DESELECT')
    model.data.transform(model.matrix_world)
    model.matrix_world = Matrix.Identity(4)
    model.data.transform(Matrix.Scale(100, 4))
    model.select_set(True)
    bpy.context.view_layer.objects.active = model
    bpy.context.scene.unit_settings.scale_length = .01
    bpy.ops.export_scene.fbx(filepath=str(folder / (name + '.fbx')), use_selection=True, object_types={'MESH'},
                            apply_unit_scale=True, apply_scale_options='FBX_SCALE_UNITS', axis_forward='-Y', axis_up='Z',
                            mesh_smooth_type='FACE', add_leaf_bones=False, bake_anim=False)
    profiles.append(dict(id=eid, inches=inches))

display = BASE / 'display.json'
if not display.exists():
    display.write_text(json.dumps(dict(schemaVersion=1, products=profiles), indent=2), encoding='utf-8')
print('MIRAS_TELEVISIONS_READY')
