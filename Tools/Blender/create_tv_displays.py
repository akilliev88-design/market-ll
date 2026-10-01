"""TV showroom fixtures. Export empty furniture; preview TVs are separate objects."""
import json
import sys
from pathlib import Path

import bpy
from mathutils import Matrix

sys.path.insert(0, str(Path(__file__).resolve().parent))
import create_store_kit as k

k.BASE = k.ROOT / 'AssetInbox/Environment/Stores'
k.PREVIEWS = k.ROOT / 'Docs/Images/Stores/Televisions'

SPECS = [
    ('tv_wall_4800', 'Isikli televizyon duvari', 4.8, .65, 2.55, [.48, 1.45], False),
    ('tv_plinth_2400', 'Tek yuz televizyon podyumu', 2.4, .75, .60, [.60], False),
    ('tv_island_3000', 'Cift yuz televizyon adasi', 3.0, 1.4, .65, [.65], True),
]


def preview(path, model, camera, target):
    # The display units contain no baked-in brand, technology or product name.
    w, d, h, levels, double = json.loads(model['preview_spec'])
    dark = bpy.data.materials['MI_TV_Anthracite']
    screen = k.mat('PreviewOnly_Screen', (.02, .04, .06), 0, .17)
    shader = screen.node_tree.nodes.get('Principled BSDF')
    noise = screen.node_tree.nodes.new('ShaderNodeTexNoise')
    noise.inputs['Scale'].default_value = 3.4
    ramp = screen.node_tree.nodes.new('ShaderNodeValToRGB')
    colors = [(0, (.005, .025, .09, 1)), (.32, (.02, .21, .65, 1)),
              (.53, (.03, .75, .39, 1)), (.72, (.95, .55, .04, 1)),
              (1, (.62, .04, .23, 1))]
    ramp.color_ramp.elements.remove(ramp.color_ramp.elements[1])
    for index, (pos, color) in enumerate(colors):
        e = ramp.color_ramp.elements[0] if index == 0 else ramp.color_ramp.elements.new(pos)
        e.position = pos
        e.color = color
    screen.node_tree.links.new(noise.outputs['Fac'], ramp.inputs['Fac'])
    screen.node_tree.links.new(ramp.outputs['Color'], shader.inputs['Base Color'])
    screen.node_tree.links.new(ramp.outputs['Color'], shader.inputs['Emission Color'])
    shader.inputs['Emission Strength'].default_value = .8
    for level, z in enumerate(levels):
        n = 3 if w > 4 else 2
        pitch = (w - (.65 if w > 4 else .20)) / n
        tw = min(1.16, pitch - .12)
        th = tw * 9 / 16
        for side in range(2 if double else 1):
            sign = -1 if side == 0 else 1
            y = sign * (.16 if double else .04)
            for i in range(n):
                x = (i - (n - 1) / 2) * pitch
                center = z + th / 2 + .055
                k.cube('PreviewOnly_TV', (x, y, center), (tw, .045, th), dark, .006, False)
                k.cube('PreviewOnly_Picture', (x, y + sign * .024, center),
                       (tw - .025, .002, th - .025), screen, .001, False)
                for foot in (-1, 1):
                    k.cube('PreviewOnly_TVFoot', (x + foot * tw * .32, y, z + .026),
                           (.07, .22, .05), dark, .003, False)
    k.cube('PreviewFloor', (0, 0, -.04), (30, 30, .06), k.mat('PreviewFloor', (.23, .25, .27), 0, .27), 0, False)
    bpy.ops.object.camera_add(location=camera)
    c = bpy.context.object
    c.data.lens = 48
    k.look_at(c, target)
    bpy.context.scene.camera = c
    for loc, energy, size in [((1, -4, 5), 1600, 4), ((-4, -1, 3), 1100, 3), ((2, 3, 4), 1400, 3)]:
        bpy.ops.object.light_add(type='AREA', location=loc)
        light = bpy.context.object
        light.data.energy = energy
        light.data.size = size
        k.look_at(light, target)
    s = bpy.context.scene
    s.render.engine = 'CYCLES'
    s.cycles.samples = 32
    s.cycles.use_denoising = True
    s.render.resolution_x = 1400
    s.render.resolution_y = 1000
    s.render.resolution_percentage = 100
    s.world.color = (.055, .065, .075)
    s.view_settings.look = 'AgX - Medium High Contrast'
    s.use_nodes = True
    nodes = s.compositing_node_group if hasattr(s, 'compositing_node_group') else None
    # Bloom comes from actual emissive trim and the render's optical glare.
    if nodes is None and hasattr(s, 'node_tree'):
        nodes = s.node_tree
    if nodes:
        nodes.nodes.clear()
        source = nodes.nodes.new('CompositorNodeRLayers')
        glare = nodes.nodes.new('CompositorNodeGlare')
        glare.glare_type = 'FOG_GLOW'
        output = nodes.nodes.new('CompositorNodeComposite')
        nodes.links.new(source.outputs['Image'], glare.inputs['Image'])
        nodes.links.new(glare.outputs['Image'], output.inputs['Image'])
    s.render.filepath = str(path / 'preview.png')
    bpy.ops.render.render(write_still=True)


k.preview = preview
for eid, label, w, d, h, levels, double in SPECS:
    k.reset()
    bpy.context.scene.unit_settings.system = 'METRIC'
    bpy.context.scene.unit_settings.scale_length = 1
    steel = k.mat('MI_TV_BrushedAluminium', (.55, .58, .61), .85, .25)
    white = k.mat('MI_TV_Pearl', (.72, .74, .76), .10, .31)
    dark = k.mat('MI_TV_Anthracite', (.018, .022, .028), .25, .38)
    led = k.mat('MI_TV_LED', (.8, .92, 1), 0, .24, emission=(.8, .92, 1))
    panel = k.mat('MI_TV_BlankPanel', (.075, .105, .14), .10, .38)
    parts = []
    def box(name, at, size, mat=white, bevel=.004):
        o = k.cube(name, at, size, mat, bevel)
        parts.append(o)
        return o
    wall = len(levels) == 2
    usable = w - (.64 if wall else .16)
    front = -.22 if wall else (-.64 if double else -.31)
    depth = .40 if wall else (.58 if double else .61)
    box('RecessedKick', (0, .015, .05), (w - .10, d - .08, .10), dark)
    # Keep cabinet and deck surfaces apart; coplanar faces flicker in Unreal.
    box('LowerCabinet', (0, 0, .25 if wall else (h + .06) / 2),
        (w - .60 if wall else w, d, .40 if wall else h - .14), white, .009)
    for x in (-w / 2 + .10, w / 2 - .10):
        for y in (-d / 2 + .10, d / 2 - .10):
            box('LevellingFoot', (x, y, .03), (.07, .07, .06), dark)
    if wall:
        box('BackPanel', (0, .285, (h + .454) / 2), (w - .60, .08, h - .454), white)
        for x in (-w / 2 + .15, w / 2 - .15):
            box('SideTower', (x, 0, h / 2), (.30, d, h), steel, .009)
            box('BlankCampaignPanel', (x, -d / 2 - .004, 1.36), (.25, .01, 2.12), panel)
            box('TowerLight', (x - (.137 if x > 0 else -.137), -d / 2 - .008, 1.36),
                (.009, .008, 2.14), led, .001)
        box('Header', (0, .01, h - .114), (usable, d - .04, .216), dark)
        box('HeaderBlankFace', (0, -d / 2 + .01, h - .11), (usable - .04, .01, .17), panel)
        box('HeaderLight', (0, -d / 2 + .006, h - .218), (usable - .008, .013, .014), led, .001)
        # Perforated service channels and covered cable access behind each TV bay.
        for x in (-1.3, 0, 1.3):
            box('CableChannel', (x, .231, 1.38), (.07, .012, 1.88), dark)
            for z in (.73, 1.72):
                box('CablePort', (x, .219, z), (.10, .012, .055), steel)
    else:
        for sign in (-1, 1) if double else (-1,):
            box('BlankFrontPanel', (0, sign * (d / 2 + .003), h / 2), (w - .14, .006, h - .23), panel)
        if double:
            box('CenterCableSpine', (0, 0, h + .06), (w - .16, .08, .12), dark)
    for z in levels:
        box('DisplayDeck', (0, -.015 if wall else 0, z - .018), (usable, d - .07, .036), steel)
        for sign in (-1, 1) if double else (-1,):
            y = sign * (d / 2 + .016)
            box('TicketRail', (0, y, z - .062), (usable, .024, .078), dark)
            box('LEDTrim', (0, y - sign * .006, z - .109), (usable - .008, .014, .012), led, .001)
    name = 'SM_' + ''.join(word.title() for word in eid.split('_'))
    model = k.join(parts, name, eid, (w * 1000, d * 1000, h * 1000))
    model['preview_spec'] = json.dumps([w, d, h, levels, double])
    collisions = [k.collision(name, 0, (0, 0, .24 if wall else h / 2), (w, d, .48 if wall else h))]
    if wall:
        collisions += [k.collision(name, 1, (0, .285, h / 2), (w, .08, h))]
        for i, x in enumerate((-w / 2 + .15, w / 2 - .15)):
            collisions.append(k.collision(name, i + 2, (x, 0, h / 2), (.30, d, h)))
        collisions.append(k.collision(name, 4, (0, -.015, 1.432), (usable, d - .07, .036)))
    zones = [dict(id=f'front_l{i:02d}', face='front', level=i, centerCm=[0, 0, z * 100],
                  usableWidthCm=usable * 100, usableDepthCm=depth * 100,
                  clearanceHeightCm=89 if wall else 125) for i, z in enumerate(levels)]
    if double:
        zones += [dict(zones[0], id='back_l00', face='back')]
    metadata = dict(schemaVersion=1, id=eid, displayName=label, mesh=name + '.fbx',
                    unrealMesh=f'/Game/Stores/Equipment/{name}.{name}', family='electronics',
                    dimensionsMm=dict(width=model.dimensions.x * 1000, depth=model.dimensions.y * 1000, height=model.dimensions.z * 1000),
                    origin='floor_center', frontAxis='-Y', materials=[m.name for m in model.data.materials],
                    collision=dict(policy='custom_ucx', pieces=len(collisions)), zones=zones, checkouts=0,
                    planogram=dict(doubleSided=double, widthCm=usable * 100, depthCm=depth * 100,
                                   frontY=front * 100, meshYaw=180, signOnTop=False,
                                   signZ=(h - .11) * 100 if wall else 17,
                                   signY=-d * 50 - 1, signWidthCm=usable * 100,
                                   railFrontY=d * 50 + 2.8, railAboveTopZ=-6.2),
                    televisionDisplay=True)
    camera = (w * .80, -w * 1.28 - 1.5, 3.2 if wall else 2.7)
    k.save_asset(eid, name, model, collisions, metadata, camera, (0, 0, 1.25 if wall else .70))
    # Export only furniture and UCX in cm; preview TVs stay in the editable .blend.
    bpy.ops.object.select_all(action='DESELECT')
    for o in [model, *collisions]:
        o.data.transform(o.matrix_world)
        o.matrix_world = Matrix.Identity(4)
        o.data.transform(Matrix.Scale(100, 4))
        o.select_set(True)
    bpy.context.view_layer.objects.active = model
    bpy.context.scene.unit_settings.scale_length = .01
    bpy.ops.export_scene.fbx(filepath=str(k.BASE / eid / (name + '.fbx')), use_selection=True,
                            object_types={'MESH'}, apply_unit_scale=True, apply_scale_options='FBX_SCALE_UNITS',
                            axis_forward='-Y', axis_up='Z', mesh_smooth_type='FACE', add_leaf_bones=False, bake_anim=False)
print('MIRAS_TV_DISPLAYS_READY')
