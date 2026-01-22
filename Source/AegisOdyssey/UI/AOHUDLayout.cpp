// Fill out your copyright notice in the Description page of Project Settings.


#include "AOHUDLayout.h"
#include "Input/CommonUIInputTypes.h"
#include "AOActivatableWidget.h"
#include "NativeGameplayTags.h"
#include "CommonUIExtensions.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Character/AOVMPawnComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHUDLayout)


UAOHUDLayout::UAOHUDLayout(const FObjectInitializer& InObjectInitializer)
{
	
}

void UAOHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(AOGameplayTags::UI_ACTION_ESCAPE), false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));
	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(AOGameplayTags::UI_ACTION_INVENTORY), false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleInventoryMenuAction)));

}

void UAOHUDLayout::HandleEscapeAction()
{
	if (ensure(!EscapeMenuClass.IsNull()))
	{
		UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(GetOwningLocalPlayer() , AOGameplayTags::UI_LAYER_MENU , EscapeMenuClass);
	}
}

void UAOHUDLayout::HandleInventoryMenuAction()
{
	if (ensure(!InventoryMenuClass.IsNull()))
	{
		UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(GetOwningLocalPlayer() , AOGameplayTags::UI_LAYER_MENU , InventoryMenuClass);
	}
}

UMVVM_HUD* UAOHUDLayout::GetHUDViewModel() const
{
	if (APlayerController* SourcePC = GetOwningLocalPlayer()->GetPlayerController(GetWorld()))
	{
		if (APawn* SourcePawn = SourcePC->GetPawn())
		{
			if (UAOVMPawnComponent* HUDVMComp = SourcePawn->FindComponentByClass<UAOVMPawnComponent>())
			{
				if (UMVVM_HUD* HUDVM = HUDVMComp->GetCharacterHUDViewModel())
				{
					return HUDVM;
				}
			}
		}
	}
	return nullptr;
}
