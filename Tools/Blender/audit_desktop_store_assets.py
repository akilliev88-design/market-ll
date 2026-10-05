"""Audit authored source geometry; export only model and UCX in centimetres."""
import bpy, sys, json, math
from pathlib import Path
from mathutils import Matrix
from mathutils.bvhtree import BVHTree
root=Path(sys.argv[sys.argv.index('--')+1]).resolve()
base=root/'AssetInbox/Environment/Stores/Desktop'
entries=json.loads((base/'manifest.json').read_text(encoding='utf-8'))['assets']
def overlap_area(poly,triangle):
    cross=lambda a,b,c:(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
    orient=1 if cross(*triangle)>0 else -1
    for j in range(3):
        a,b=triangle[j],triangle[(j+1)%3]; new=[]
        for i,p in enumerate(poly):
            q=poly[(i+1)%len(poly)];dp=orient*cross(a,b,p);dq=orient*cross(a,b,q)
            if dp>=-1e-12:new.append(p)
            if (dp>=-1e-12)!=(dq>=-1e-12):
                t=dp/(dp-dq);new.append((p[0]+t*(q[0]-p[0]),p[1]+t*(q[1]-p[1])))
        poly=new
        if not poly:break
    return abs(sum(poly[i][0]*poly[(i+1)%len(poly)][1]-poly[(i+1)%len(poly)][0]*poly[i][1] for i in range(len(poly))))/2 if poly else 0
def coincident(mesh):
    mesh.calc_loop_triangles();vs=[v.co.copy() for v in mesh.vertices];ts=list(mesh.loop_triangles)
    tris=[tuple(t.vertices) for t in ts]
    ns=[(vs[t[1]]-vs[t[0]]).cross(vs[t[2]]-vs[t[0]]).normalized() for t in tris]
    tree=BVHTree.FromPolygons(vs,tris,all_triangles=True,epsilon=.000001)
    hits=[]
    for i,j in tree.overlap(tree):
        if i>=j or ts[i].polygon_index==ts[j].polygon_index:continue
        if ns[i].dot(ns[j])<.999999:continue
        if abs(ns[i].dot(vs[tris[i][0]]-vs[tris[j][0]]))>.000003:continue
        ax=max(range(3),key=lambda a:abs(ns[i][a]));keep=[a for a in range(3) if a!=ax]
        project=lambda t:[tuple(vs[v][a] for a in keep) for v in t]
        if overlap_area(project(tris[i]),project(tris[j]))>1e-8:hits.append((ts[i].polygon_index,ts[j].polygon_index))
    return sorted(set(hits))
def separate_faces(obj,faces):
    old=obj.data;verts=[];polys=[];uvs={uv.name:[] for uv in old.uv_layers};slots=[];smooth=[];weights=[];group_names=[g.name for g in obj.vertex_groups]
    for p in old.polygons:
        shift=p.normal*.0008 if p.index in faces else p.normal*0
        indices=[]
        for li in p.loop_indices:
            indices.append(len(verts));verts.append(old.vertices[old.loops[li].vertex_index].co+shift)
            weights.append([(g.group,g.weight) for g in old.vertices[old.loops[li].vertex_index].groups])
            for uv in old.uv_layers:uvs[uv.name].append(tuple(uv.data[li].uv))
        polys.append(indices);slots.append(p.material_index);smooth.append(p.use_smooth)
    new=bpy.data.meshes.new(old.name+'_Separated');new.from_pydata(verts,[],polys);new.update()
    for m in old.materials:new.materials.append(m)
    for p,slot,sm in zip(new.polygons,slots,smooth):p.material_index=slot;p.use_smooth=sm
    for name,values in uvs.items():
        uv=new.uv_layers.new(name=name)
        for loop,v in zip(uv.data,values):loop.uv=v
    obj.data=new
    obj.vertex_groups.clear()
    for name in group_names:obj.vertex_groups.new(name=name)
    for vertex,groups in enumerate(weights):
        for group,weight in groups:obj.vertex_groups[group].add([vertex],weight,'REPLACE')
reports=[]
for entry in entries:
    folder=root/entry['folder'];info=json.loads((folder/'equipment.json').read_text(encoding='utf-8'))
    source=folder/info['sourceBlend'];bpy.ops.wm.open_mainfile(filepath=str(source))
    model=bpy.data.objects.get(Path(info['mesh']).stem)
    if not model: raise RuntimeError('Missing model '+entry['id'])
    bpy.context.view_layer.objects.active=model
    if not all(math.isfinite(c) for v in model.data.vertices for c in v.co): raise RuntimeError('Nonfinite '+entry['id'])
    before=coincident(model.data);fixed=0
    for attempt in range(4):
        hits=coincident(model.data)
        if not hits:break
        selected={b for a,b in hits};separate_faces(model,selected);fixed+=len(selected)
    after=coincident(model.data)
    if after:raise RuntimeError('Unresolved coplanar faces '+entry['id'])
    # Preserve the original delivery copy; repaired source has its own path.
    source_out=folder/'Source'/('Game_'+entry['id']+'.blend');source_out.parent.mkdir(exist_ok=True)
    if fixed:bpy.ops.wm.save_as_mainfile(filepath=str(source_out))
    objects=[model]+[o for o in bpy.data.objects if o.name.startswith('UCX_'+Path(info['mesh']).stem+'_')]
    collision_count=len(objects)-1
    bpy.ops.object.select_all(action='DESELECT')
    for o in objects:
        o.hide_set(False);o.hide_viewport=False;o.data.transform(o.matrix_world);o.matrix_world=Matrix.Identity(4);o.data.transform(Matrix.Scale(100,4));o.select_set(True)
    bpy.context.view_layer.objects.active=model;bpy.context.scene.unit_settings.scale_length=.01
    # Packed images need external paths so Unreal's FBX importer can find the texture.
    textures=folder/'Textures';textures.mkdir(exist_ok=True)
    for im in bpy.data.images:
        if im.packed_file:
            im.filepath_raw=str(textures/(Path(im.filepath).name or (im.name+'.png')))
            im.save();im.unpack(method='USE_LOCAL')
    bpy.ops.export_scene.fbx(filepath=str(folder/info['mesh']),use_selection=True,object_types={'MESH'},apply_unit_scale=True,apply_scale_options='FBX_SCALE_UNITS',axis_forward='-Y',axis_up='Z',use_mesh_modifiers=True,mesh_smooth_type='FACE',add_leaf_bones=False,bake_anim=False,path_mode='COPY',embed_textures=True)
    info['collision']={'policy':'custom_ucx','pieces':collision_count}
    info['geometryAudit']={'overlapPairsBefore':len(before),'shiftedFaces':fixed,'overlapPairsAfter':0}
    (folder/'equipment.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
    reports.append(dict(id=entry['id'],collisionPieces=collision_count,overlapPairsBefore=len(before),shiftedFaces=fixed,overlapPairsAfter=0))
    (base/'geometry-audit.json').write_text(json.dumps(dict(assets=reports,completed=len(reports),total=len(entries)),indent=2))
    print('DESKTOP_AUDITED '+entry['id']+' corrected='+str(fixed),flush=True)
print('DESKTOP_GEOMETRY_PASSED='+str(len(reports)),flush=True)
