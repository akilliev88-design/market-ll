"""Validate the first imported environment fixture in Unreal Engine."""

import unreal


SPECS = [
    ("/Game/Environment/Shelves/Gondola_1200/SM_Gondola_1200.SM_Gondola_1200", (120.0, 90.0, 160.0), 5, 3),
    ("/Game/Environment/StoreKit/WallShelf_2400/SM_WallShelf_2400.SM_WallShelf_2400", (240.0, 51.8, 222.0), 4, 2),
    ("/Game/Environment/StoreKit/BulkIsland_1600/SM_BulkIsland_1600.SM_BulkIsland_1600", (162.0, 101.7, 173.0), 7, 2),
]


def fail(message):
    raise RuntimeError(f"MIRAS_ENVIRONMENT_VALIDATION_ERROR={message}")


for mesh_path, dimensions, material_minimum, collision_minimum in SPECS:
    mesh = unreal.load_asset(mesh_path)
    if mesh is None:
        fail(f"mesh missing: {mesh_path}")
    bounds = mesh.get_bounding_box()
    size = bounds.max - bounds.min
    for axis, actual, target in zip("XYZ", (size.x, size.y, size.z), dimensions):
        if abs(actual - target) > 1.5:
            fail(f"{mesh_path} {axis} size is {actual:.2f} cm, expected {target:.2f} cm")
    materials = mesh.get_editor_property("static_materials")
    if len(materials) < material_minimum:
        fail(f"{mesh_path} material count {len(materials)} < {material_minimum}")
    aggregate = mesh.get_editor_property("body_setup").get_editor_property("agg_geom")
    collision_count = sum(len(aggregate.get_editor_property(field)) for field in ("box_elems", "sphere_elems", "sphyl_elems", "convex_elems"))
    if collision_count < collision_minimum:
        fail(f"{mesh_path} collision count {collision_count} < {collision_minimum}")
    unreal.log(f"MIRAS_ENVIRONMENT_VALIDATION_OK={mesh_path}: {size.x:.1f}x{size.y:.1f}x{size.z:.1f} cm, {len(materials)} materials, {collision_count} collisions")

ceiling_path = "/Game/Environment/StoreKit/CeilingBay_6000/SM_CeilingBay_6000.SM_CeilingBay_6000"
ceiling = unreal.load_asset(ceiling_path)
if ceiling is None:
    fail("ceiling mesh missing")
ceiling_size = ceiling.get_bounding_box().max - ceiling.get_bounding_box().min
if abs(ceiling_size.x - 600.0) > 2 or abs(ceiling_size.y - 600.0) > 2:
    fail(f"ceiling footprint is {ceiling_size.x:.1f}x{ceiling_size.y:.1f} cm")
unreal.log(f"MIRAS_ENVIRONMENT_VALIDATION_OK={ceiling_path}: {ceiling_size.x:.1f}x{ceiling_size.y:.1f}x{ceiling_size.z:.1f} cm")
