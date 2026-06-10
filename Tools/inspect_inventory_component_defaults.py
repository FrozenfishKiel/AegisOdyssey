import unreal


ASSET_PATHS = [
    "/Game/Blueprints/CBP_EnemyCharacter",
    "/Game/Games/Character/BP_Character_Default",
    "/Game/Blueprints/CBP_SandboxCharacter",
]


def dump_component_slot_count(asset_path: str) -> None:
    bp_class = unreal.EditorAssetLibrary.load_blueprint_class(asset_path)
    if not bp_class:
        unreal.log_warning(f"[InspectInventoryDefaults] Failed to load blueprint class: {asset_path}")
        return

    cdo = unreal.get_default_object(bp_class)
    if not cdo:
        unreal.log_warning(f"[InspectInventoryDefaults] Failed to load CDO: {asset_path}")
        return

    unreal.log(f"[InspectInventoryDefaults] Asset={asset_path}")

    for component_name in ["CharacterBackPackComponent", "CharacterQuickBar", "CharacterFormalEquipmentSlotInventoryComponent"]:
        try:
            component = cdo.get_editor_property(component_name)
        except Exception as exc:
            unreal.log_warning(f"[InspectInventoryDefaults]   {component_name}: property read failed: {exc}")
            continue

        if not component:
            unreal.log_warning(f"[InspectInventoryDefaults]   {component_name}: None")
            continue

        try:
            num_slots = component.get_editor_property("NumSlots")
        except Exception:
            num_slots = "<no NumSlots>"

        unreal.log(f"[InspectInventoryDefaults]   {component_name}: class={component.get_class().get_name()} NumSlots={num_slots}")


for path in ASSET_PATHS:
    dump_component_slot_count(path)
