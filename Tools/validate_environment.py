"""Validate the first imported environment fixture in Unreal Engine."""

import unreal


MESH_PATH = "/Game/Environment/Shelves/Gondola_1200/SM_Gondola_1200.SM_Gondola_1200"


def fail(message):
    raise RuntimeError(f"MIRAS_ENVIRONMENT_VALIDATION_ERROR={message}")


mesh = unreal.load_asset(MESH_PATH)
if mesh is None:
    fail("mesh missing")

bounds = mesh.get_bounding_box()
size = bounds.max - bounds.min
expected = unreal.Vector(120.0, 90.0, 160.0)
for axis, actual, target in zip("XYZ", (size.x, size.y, size.z), (expected.x, expected.y, expected.z)):
    if abs(actual - target) > 0.75:
        fail(f"{axis} size is {actual:.2f} cm, expected {target:.2f} cm")

materials = mesh.get_editor_property("static_materials")
if len(materials) < 5:
    fail(f"material slot count is {len(materials)}, expected at least 5")

body_setup = mesh.get_editor_property("body_setup")
if body_setup is None:
    fail("body setup is missing")
aggregate = body_setup.get_editor_property("agg_geom")
collision_count = sum(
    len(aggregate.get_editor_property(field))
    for field in ("box_elems", "sphere_elems", "sphyl_elems", "convex_elems")
)
if collision_count < 3:
    fail(f"simple collision count is {collision_count}, expected at least 3")

unreal.log(
    "MIRAS_ENVIRONMENT_VALIDATION_OK="
    f"{size.x:.1f}x{size.y:.1f}x{size.z:.1f} cm, "
    f"{len(materials)} materials, {collision_count} collision primitives"
)
