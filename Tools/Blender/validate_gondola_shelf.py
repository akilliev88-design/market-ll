"""Validate the generated gondola source scene and metadata inside Blender."""

import json
import sys
from pathlib import Path

import bpy


def fail(message):
    print(f"SIM_VALIDATION_ERROR={message}")
    raise SystemExit(1)


def near(actual, expected, tolerance=0.003):
    return abs(actual - expected) <= tolerance


def main():
    args = sys.argv
    if "--" not in args or len(args) <= args.index("--") + 1:
        fail("project root argument missing")
    root = Path(args[args.index("--") + 1]).resolve()
    asset_dir = root / "AssetInbox" / "Environment" / "Shelves" / "Gondola_1200"
    metadata = json.loads((asset_dir / "equipment.json").read_text(encoding="utf-8"))

    model = bpy.data.objects.get("SM_Gondola_1200")
    if model is None:
        fail("SM_Gondola_1200 missing")
    expected = (1.2, 0.9, 1.6)
    if not all(near(actual, target) for actual, target in zip(model.dimensions, expected)):
        fail(f"dimensions={tuple(round(value, 4) for value in model.dimensions)}")
    if model.location.length > 0.001:
        fail(f"origin location={tuple(round(value, 4) for value in model.location)}")
    if any(abs(value - 1.0) > 0.001 for value in model.scale):
        fail(f"unapplied scale={tuple(model.scale)}")

    material_names = {slot.material.name for slot in model.material_slots if slot.material}
    required = {
        "MI_Shelf_PaintedMetal",
        "MI_Shelf_BackPanel",
        "MI_Shelf_WarmWood",
        "MI_Shelf_PriceRail",
        "MI_Shelf_Rubber",
    }
    if not required.issubset(material_names):
        fail(f"materials={sorted(material_names)}")

    collisions = [obj for obj in bpy.data.objects if obj.name.startswith("UCX_SM_Gondola_1200_")]
    if len(collisions) != 3:
        fail(f"collision count={len(collisions)}")
    if len(metadata.get("shelfZones", [])) != 8:
        fail("metadata requires 8 shelf zones")
    if metadata.get("frontAxis") != "-Y":
        fail("front axis must be -Y")

    for required_file in (
        asset_dir / "SM_Gondola_1200.fbx",
        asset_dir / "Source" / "Gondola_1200.blend",
        root / "Docs" / "Images" / "gondola_1200_preview.png",
    ):
        if not required_file.exists() or required_file.stat().st_size < 1024:
            fail(f"missing or empty file={required_file}")

    print("SIM_VALIDATION_OK=SM_Gondola_1200 1200x900x1600 mm, 5 materials, 3 UCX, 8 zones")


if __name__ == "__main__":
    main()
