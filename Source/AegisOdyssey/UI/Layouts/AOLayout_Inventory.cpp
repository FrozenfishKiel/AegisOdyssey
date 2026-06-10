// Fill out your copyright notice in the Description page of Project Settings.

#include "AOLayout_Inventory.h"

#include "Input/CommonUIInputTypes.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Interaction/Session/AOInteractionSessionModel.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryPageUI.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOLayout_Inventory)

void UAOLayout_Inventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RegisterUIActionBinding(FBindUIActionArgs(
		FUIActionTag::ConvertChecked(AOGameplayTags::UI_ACTION_INVENTORY),
		false,
		FSimpleDelegate::CreateUObject(this, &ThisClass::HandleInventoryMenuAction)));
}

void UAOLayout_Inventory::NativeOnActivated()
{
	Super::NativeOnActivated();
	RefreshInventoryPageContext();
}

void UAOLayout_Inventory::HandleInteractionSessionChanged(UAOInteractionSessionModel* NewSessionModel)
{
	Super::HandleInteractionSessionChanged(NewSessionModel);
	RefreshInventoryPageContext();
}

void UAOLayout_Inventory::HandleInventoryMenuAction()
{
	DeactivateWidget();
}

void UAOLayout_Inventory::RefreshInventoryPageContext()
{
	if (InventoryPageWidget != nullptr)
	{
		InventoryPageWidget->RefreshInventoryPageContexts();
	}
}
