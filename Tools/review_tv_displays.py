"""Transient Unreal showroom render. Never saves a map, catalog or campaign."""
import time
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir()).resolve()
world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def mesh(path, at, yaw=0, scale=None):
    asset = unreal.load_asset(path)
    assert asset, path
    actor = subsystem.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*at), unreal.Rotator(yaw=yaw))
    component = actor.static_mesh_component
    component.set_static_mesh(asset)
    if scale: actor.set_actor_scale3d(unreal.Vector(*scale))
    return actor


mesh('/Game/Stores/Equipment/SM_TvWall4800', (0, 250, 0), 180)
mesh('/Game/Stores/Equipment/SM_TvPlinth2400', (-180, -25, 0), 180)
mesh('/Game/Stores/Equipment/SM_TvIsland3000', (210, -140, 0), 180)
for inches, x, z in [(43, -138, 48), (55, 0, 48), (43, 138, 48),
                       (55, -135, 145), (65, 25, 145)]:
    mesh(f'/Game/Stores/Televisions/SM_Television{inches}', (x, 247, z), -90)
for inches, at, yaw in [(32, (-237, -25, 60), -90), (55, (-110, -25, 60), -90),
                         (43, (120, -160, 65), -90), (75, (260, -160, 65), -90),
                         (65, (200, -110, 65), 90)]:
    mesh(f'/Game/Stores/Televisions/SM_Television{inches}', at, yaw)
floor = mesh('/Engine/BasicShapes/Cube', (0, 0, -5), 0, (22, 22, .10))
for loc, intensity, size, rot in [((0, -200, 420), 18000, 600, (-65, 0, 90)),
                                   ((-400, -400, 280), 12000, 500, (-25, 0, 45))]:
    light = subsystem.spawn_actor_from_class(unreal.RectLight, unreal.Vector(*loc), unreal.Rotator(pitch=rot[0], yaw=rot[2]))
    component = light.get_component_by_class(unreal.RectLightComponent)
    component.set_intensity(intensity)
    component.set_source_width(size)
    component.set_source_height(size)
camera = subsystem.spawn_actor_from_class(unreal.CameraActor, unreal.Vector(720, -1080, 470), unreal.Rotator(pitch=-14.3, yaw=122.3))
camera.camera_component.set_field_of_view(48)
unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(camera.get_actor_location(), camera.get_actor_rotation())
unreal.SystemLibrary.execute_console_command(world, 'r.BloomQuality 5')
output = root / 'Saved/Screenshots/Televisions/UnrealShowroom.png'
output.parent.mkdir(parents=True, exist_ok=True)
started = time.monotonic()
captured = False
handle = None


def tick(delta):
    global captured
    elapsed = time.monotonic() - started
    if not captured and elapsed > 25:
        unreal.AutomationLibrary.take_high_res_screenshot(1400, 1000, str(output), camera=camera)
        captured = True
    if captured and elapsed > 35:
        unreal.log('MIRAS_TV_RENDER_COMPLETE=' + str(output))
        unreal.unregister_slate_post_tick_callback(handle)
        unreal.SystemLibrary.quit_editor()


handle = unreal.register_slate_post_tick_callback(tick)
