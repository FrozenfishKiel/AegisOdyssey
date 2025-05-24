// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerState.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOPlayerState)
const FName AAOPlayerState::NAME_AOAbilityReady("AOAbilitiesReady");
AAOPlayerController* AAOPlayerState::GetAOPlayerController() const
{
	return Cast<AAOPlayerController>(GetOwner());
}
