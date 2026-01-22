// Fill out your copyright notice in the Description page of Project Settings.


#include "AOInventoryUI.h"
#include "AegisOdyssey/Character/AOVMPawnComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryUI)

UMVVM_InventoryMenu* UAOInventoryUI::GetInventoryViewModel() const
{
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (APlayerController* SourcePC = LocalPlayer->GetPlayerController(GetWorld()))
		{
			if (APawn* ControlledPawn = SourcePC->GetPawn())
			{
				if (UAOVMPawnComponent* ViewModelPawn = ControlledPawn->FindComponentByClass<UAOVMPawnComponent>())
				{
					return ViewModelPawn->GetCharacterInventoryViewModel();
				}
			}
		}
	}
	return nullptr;
}
