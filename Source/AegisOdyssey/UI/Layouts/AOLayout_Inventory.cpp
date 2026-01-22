// Fill out your copyright notice in the Description page of Project Settings.


#include "AOLayout_Inventory.h"
#include "Input/CommonUIInputTypes.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOLayout_Inventory)

void UAOLayout_Inventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(AOGameplayTags::UI_ACTION_INVENTORY), false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleInventoryMenuAction)));

}

//在Layout已经弹到界面的时候只能选择关闭该Widget
void UAOLayout_Inventory::HandleInventoryMenuAction()
{
	DeactivateWidget();
}
