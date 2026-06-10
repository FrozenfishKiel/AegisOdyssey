// Fill out your copyright notice in the Description page of Project Settings.

#include "AOGameplayTags.h"

namespace AOGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "Actor/component has initially spawned");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "Required data is available");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "Available data has been initialized");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "Actor/component is ready for gameplay");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Move, "Input.Move", "Move input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_LookUp, "Input.LookUp", "Look input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_LightAttack, "Input.LightAttack", "Light attack input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Interact, "Input.Interact", "Interact input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Jump, "Input.Jump", "Jump input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Roll, "Input.Roll", "Roll input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Sprint, "Input.Sprint", "Sprint input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_CancelSprint, "Input.CancelSprint", "Cancel sprint input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_CancelJump, "Input.CancelJump", "Cancel jump input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Block, "Input.Block", "Block input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_CancelBlock, "Input.CancelBlock", "Cancel block input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_HeavyAttack, "Input.HeavyAttack", "Heavy attack input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Hold, "Input.Hold", "Hold input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_QuickSlotSelect, "Input.QuickSlotSelect", "Quick bar slot selection input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_SkillSlot1, "Input.SkillSlot1", "Skill slot 1 input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_SkillSlot2, "Input.SkillSlot2", "Skill slot 2 input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_SkillSlot3, "Input.SkillSlot3", "Skill slot 3 input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_SkillSlot4, "Input.SkillSlot4", "Skill slot 4 input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Game, "UI.Layer.Game", "Game UI layer");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_LAYER_MENU, "UI.Layer.Menu", "Menu UI layer");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_ACTION_ESCAPE, "UI.Action.Escape", "Escape action");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_ACTION_INVENTORY, "UI.Action.Inventory", "Inventory action");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HUD_Slot_QuickBar, "HUD.Slot.QuickBar", "Quick bar slot");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Intent_Attack, "AI.Intent.Attack", "AI attack intent");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Intent_Strafe, "AI.Intent.Strafe", "AI strafe intent");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Intent_Roll, "AI.Intent.Roll", "AI roll intent");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Tactical_HasTarget, "AI.Tactical.HasTarget", "AI tactical state: current target exists");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Tactical_NoTarget, "AI.Tactical.NoTarget", "AI tactical state: current target is missing");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Tactical_Target_InRange, "AI.Tactical.Target.InRange", "AI tactical state: target is in current attack range");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Tactical_Target_OutOfRange, "AI.Tactical.Target.OutOfRange", "AI tactical state: target is outside current attack range");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Tactical_Target_Preparation, "AI.Tactical.Target.Preparation", "AI tactical state: target is preparing an action");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Tactical_Target_CombatWindow, "AI.Tactical.Target.CombatWindow", "AI tactical state: target exposes a combat window");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Tactical_Target_Combating, "AI.Tactical.Target.Combating", "AI tactical state: target is in an active combat chain");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Tactical_Target_Recovery, "AI.Tactical.Target.Recovery", "AI tactical state: target is recovering");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Tactical_Self_WeaponFitsCombatDistance, "AI.Tactical.Self.WeaponFitsCombatDistance", "AI tactical state: current weapon already fits combat distance");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Tactical_Self_NeedWeaponSwap, "AI.Tactical.Self.NeedWeaponSwap", "AI tactical state: current weapon no longer fits combat distance");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Tactical_Inventory_AdditiveWindow, "AI.Tactical.Inventory.AdditiveWindow", "AI tactical state: current main behavior allows additive inventory actions");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Decision_Inventory_UseItem, "AI.Decision.Inventory.UseItem", "AI unified inventory decision tag: submit inventory-use behavior to the action layer");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Event_InventoryDecision_Updated, "AI.Event.InventoryDecision.Updated", "AI inventory decision result became available or changed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Event_InventoryDecision_Cleared, "AI.Event.InventoryDecision.Cleared", "AI inventory decision result was cleared");
}
