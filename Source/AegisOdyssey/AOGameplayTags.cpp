// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameplayTags.h"

namespace AOGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Move,"Input.Move","Player Base Moving Mode");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_LookUp,"Input.LookUp","Player Base Look Mode");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_LightAttack,"Input.LightAttack","角色轻攻击");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Interact,"Input.Interact","Player begin Interact");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Jump,"Input.Jump","Player begin Jump.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Roll,"Input.Roll","Player begin Roll.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Sprint,"Input.Sprint","Player begin Sprint.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_CalcelSprint,"Input.CancelSprint","Player begin CancelSprint.");



	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Game,"UI.Layer.Game","Player Game Main UI Layer.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_LAYER_MENU,"UI.Layer.Menu","Player Game Menu UI Layer.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_ACTION_ESCAPE,"UI.Action.Escape","UI Action Escape.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_ACTION_INVENTORY,"UI.Action.Inventory","UI Action Inventory Menu.");




	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HUD_Slot_QuickBar , "HUD.Slot.QuickBar" , "QuickBarSlot");


}