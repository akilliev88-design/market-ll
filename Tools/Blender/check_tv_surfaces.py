"""Find overlapping axis-aligned, same-facing planes in the TV fixture sources."""
import json
import sys
from pathlib import Path
import bpy

root = Path(sys.argv[sys.argv.index('--') + 1]).resolve()
report = []
for eid in ('tv_wall_4800', 'tv_plinth_2400', 'tv_island_3000'):
    folder = root / 'AssetInbox/Environment/Stores' / eid
    meta = json.loads((folder / 'equipment.json').read_text())
    bpy.ops.wm.open_mainfile(filepath=str(folder / 'Source' / (eid + '.blend')))
    model = bpy.data.objects[Path(meta['mesh']).stem]
    faces = []
    for poly in model.data.polygons:
        axis = max(range(3), key=lambda i: abs(poly.normal[i]))
        if abs(poly.normal[axis]) < .99999: continue
        vertices = [model.matrix_world @ model.data.vertices[i].co for i in poly.vertices]
        other = [i for i in range(3) if i != axis]
        faces.append(dict(axis=axis, sign=1 if poly.normal[axis] > 0 else -1,
                          plane=sum(v[axis] for v in vertices) / len(vertices),
                          lo=[min(v[i] for v in vertices) for i in other],
                          hi=[max(v[i] for v in vertices) for i in other],
                          material=model.data.materials[poly.material_index].name))
    overlaps = []
    for index, a in enumerate(faces):
        for b in faces[index + 1:]:
            if a['axis'] != b['axis'] or a['sign'] != b['sign'] or abs(a['plane'] - b['plane']) > 1e-6: continue
            span = [min(a['hi'][i], b['hi'][i]) - max(a['lo'][i], b['lo'][i]) for i in range(2)]
            if min(span) < .0001 or span[0] * span[1] < .00001: continue
            # The underside sits on the floor; only outward visible planes are relevant here.
            if a['axis'] == 2 and a['sign'] == -1 and abs(a['plane']) < .001: continue
            overlaps.append(dict(axis=a['axis'], sign=a['sign'], plane=round(a['plane'], 5),
                                 areaCm2=round(span[0] * span[1] * 10000, 2), materials=[a['material'], b['material']]))
    report.append(dict(id=eid, overlaps=overlaps))
print('MIRAS_TV_COPLANAR=' + json.dumps(report))
if '--strict' in sys.argv:
    assert not any(row['overlaps'] for row in report), 'Visible coplanar surface overlap'
