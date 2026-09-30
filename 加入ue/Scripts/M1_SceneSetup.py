"""
M1_SceneSetup.py — blocks out the sky-bridge smoking scene for milestone M1.

Run in an OPEN, EMPTY level:  Tools > Execute Python Script...  (needs the
"Python Editor Script Plugin" enabled).  Everything is placed in the outliner
folder "M1" so it is easy to find, move or delete.

Units: centimetres.  Axes: +X = towards the window, +Y = right, +Z = up.
Grey-box meshes use engine basic shapes — swap in Megascans / your own meshes
afterwards, keeping the same positions.
"""
import unreal

eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
CUBE = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
PLANE = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Plane.Plane")
FOLDER = "M1"


def setp(obj, name, value):
    """Set an editor property, but don't stop the whole script if a name differs between engine versions."""
    try:
        obj.set_editor_property(name, value)
    except Exception as e:  # noqa
        unreal.log_warning("M1 setup: could not set %s on %s (%s)" % (name, obj.get_name(), e))


def spawn(cls, loc, pitch=0.0, yaw=0.0, roll=0.0, label=None):
    a = eas.spawn_actor_from_class(cls, unreal.Vector(*loc), unreal.Rotator(roll=roll, pitch=pitch, yaw=yaw))
    if label:
        a.set_actor_label(label)
    a.set_folder_path(FOLDER)
    return a


def box(label, center, size):
    """Grey-box cube: size in cm (x, y, z)."""
    a = spawn(unreal.StaticMeshActor, center, label=label)
    a.static_mesh_component.set_static_mesh(CUBE)
    a.set_actor_scale3d(unreal.Vector(size[0] / 100.0, size[1] / 100.0, size[2] / 100.0))
    return a


# ---------------------------------------------------------------- geometry
TABLE_TOP = 75.0
box("Counter_Top", (0, 0, TABLE_TOP - 2.5), (60, 400, 5))            # 60 deep, 4 m wide, top at 75 cm
box("Counter_Body", (0, 0, (TABLE_TOP - 5) / 2), (56, 400, TABLE_TOP - 5))
box("Floor", (0, 0, -1), (400, 1200, 2))
box("Ceiling", (-50, 0, 300), (300, 1200, 4))

GLASS_X = 50.0                                                       # 20 cm past the counter's far edge
glass = spawn(unreal.StaticMeshActor, (GLASS_X, 0, 150), pitch=0, yaw=0, roll=90, label="Glass_AssignTranslucentMaterial")
glass.static_mesh_component.set_static_mesh(PLANE)
glass.set_actor_rotation(unreal.Rotator(roll=0, pitch=90, yaw=0), False)  # plane faces -X (into the room)
glass.set_actor_scale3d(unreal.Vector(3.0, 12.0, 1.0))

for i, y in enumerate(range(-600, 601, 150)):                        # vertical mullions every 1.5 m
    box("Mullion_%02d" % i, (GLASS_X, y, 150), (6, 5, 300))
box("Rail_Low", (GLASS_X - 2, 0, 2), (10, 1200, 4))
box("Rail_Waist", (GLASS_X - 2, 0, 110), (8, 1200, 5))

# ---------------------------------------------------------------- placement markers (attach real assets here)
def marker(label, loc):
    return spawn(unreal.TargetPoint, loc, label=label)

marker("Place_CigPack", (-5, -30, TABLE_TOP))                        # left hand side
marker("Place_Ashtray", (-5, 32, TABLE_TOP))                         # right hand side
marker("Place_RightHandRest", (-35, 12, TABLE_TOP + 22))             # wrist target while holding the cigarette
marker("Place_Mouth", (-58, 0, TABLE_TOP + 37))                      # ~8 cm below the camera
marker("Place_ChestAshSpot", (-62, 4, TABLE_TOP + 5))

# ---------------------------------------------------------------- camera (the player's eyes)
cam = spawn(unreal.CineCameraActor, (-60, 0, TABLE_TOP + 45), pitch=-20, label="FP_EyeCamera_Reference")
cc = cam.get_cine_camera_component()
setp(cc, "current_focal_length", 24.0)
setp(cc, "current_aperture", 2.8)
fs = cc.get_editor_property("focus_settings")
setp(fs, "manual_focus_distance", 45.0)                               # focus on the hand / cigarette
setp(cc, "focus_settings", fs)

# ---------------------------------------------------------------- lights
pendant = spawn(unreal.RectLight, (-10, 0, TABLE_TOP + 95), pitch=-90, label="Light_Pendant_Warm")
rl = pendant.rect_light_component
setp(rl, "intensity_units", unreal.LightUnits.CANDELAS)
rl.set_intensity(8.0)
setp(rl, "use_temperature", True)
setp(rl, "temperature", 2700.0)
setp(rl, "source_width", 25.0)
setp(rl, "source_height", 25.0)
setp(rl, "attenuation_radius", 400.0)
setp(rl, "barn_door_angle", 60.0)
setp(rl, "barn_door_length", 15.0)

moon = spawn(unreal.DirectionalLight, (300, 0, 400), pitch=-28, yaw=190, label="Light_Moon_Cool")
ml = moon.get_component_by_class(unreal.DirectionalLightComponent)
ml.set_intensity(0.3)                                                 # lux
setp(ml, "use_temperature", True)
setp(ml, "temperature", 7500.0)
setp(ml, "atmosphere_sun_light", False)

sky = spawn(unreal.SkyLight, (0, 0, 200), label="Light_Sky")
sl = sky.get_component_by_class(unreal.SkyLightComponent)
setp(sl, "real_time_capture", True)
sl.set_intensity(0.4)

fog = spawn(unreal.ExponentialHeightFog, (0, 0, -15000), label="Fog_AerialPerspective")
fc = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
setp(fc, "fog_density", 0.0035)
setp(fc, "fog_height_falloff", 0.02)
setp(fc, "start_distance", 2000.0)                                   # keep the room itself clear
setp(fc, "fog_inscattering_luminance", unreal.LinearColor(0.08, 0.1, 0.16, 1.0))
setp(fc, "volumetric_fog", True)
setp(fc, "volumetric_fog_scattering_distribution", 0.6)
setp(fc, "volumetric_fog_extinction_scale", 1.0)

# ---------------------------------------------------------------- post process
pp = spawn(unreal.PostProcessVolume, (0, 0, 100), label="PostProcess_Global")
setp(pp, "unbound", True)
s = pp.get_editor_property("settings")

def pps(name, value):
    try:
        s.set_editor_property("override_" + name, True)
    except Exception:
        pass
    setp(s, name, value)

pps("auto_exposure_method", unreal.AutoExposureMethod.AEM_MANUAL)
pps("auto_exposure_bias", 10.0)             # tweak until the counter under the lamp reads right (roughly 9–12)
pps("bloom_intensity", 0.45)
pps("bloom_threshold", 1.0)
pps("vignette_intensity", 0.28)
pps("film_grain_intensity", 0.05)
pps("lens_flare_intensity", 0.0)
pps("dynamic_global_illumination_method", unreal.DynamicGlobalIlluminationMethod.LUMEN)
pps("reflection_method", unreal.ReflectionMethod.LUMEN)
pps("lumen_final_gather_quality", 2.0)
pps("lumen_reflection_quality", 2.0)
pps("white_temp", 6200.0)
setp(pp, "settings", s)

unreal.log("M1 scene block-out done — see outliner folder 'M1'.")
