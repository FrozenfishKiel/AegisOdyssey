// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOPlayerController)

void AAOPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);

	if (UAOAbilitySystem* AOASC = Cast<UAOAbilitySystem>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn())))
	{
		AOASC->ProcessAbilityInput(DeltaTime,bGamePaused);
	}
}
