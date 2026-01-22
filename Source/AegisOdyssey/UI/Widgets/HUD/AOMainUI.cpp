// Fill out your copyright notice in the Description page of Project Settings.


#include "AOMainUI.h"

#include "AegisOdyssey/Character/AOVMPawnComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOMainUI)

void UAOMainUI::NativeConstruct()
{
	Super::NativeConstruct();
	// 玩家确保在控制器完成创建且拥有自己的Pawn的时候才能从中获取自身的ViewModemComponent
}

UMVVM_HUD* UAOMainUI::GetMainHUDViewModel() const
{
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (APlayerController* SourcePC = LocalPlayer->GetPlayerController(GetWorld()))
		{
			if (APawn* ControlledPawn = SourcePC->GetPawn())
			{
				if (UAOVMPawnComponent* ViewModelPawn = ControlledPawn->FindComponentByClass<UAOVMPawnComponent>())
				{
					return ViewModelPawn->GetCharacterHUDViewModel();
				}
			}
		}
	}
	return nullptr;
}
