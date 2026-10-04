"""Read-only: list the existing garment material's tint parameters for the art trial."""
import unreal

base = "/Game/MetaHumans/MH_Teyze/Clothing/"
for name in ["MI_WI_DefaultGarment_M_DG_bodyShapeE_Shirt", "MI_WI_DefaultGarment_M_DG_bodyShapeE_Short"]:
    material = unreal.load_asset(base + name)
    if material:
        unreal.log("ART_CLOTH_PARAMETERS " + name + " " + str(unreal.MaterialEditingLibrary.get_vector_parameter_names(material)))
    else:
        unreal.log_error("ART_CLOTH_MISSING " + name)
