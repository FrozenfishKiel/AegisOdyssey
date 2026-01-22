// Fill out your copyright notice in the Description page of Project Settings.


#include "AOLocalPlayer.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOLocalPlayer)

void UAOLocalPlayer::SwitchController(class APlayerController* PC)
{
	Super::SwitchController(PC);

	OnControllerChanged(PlayerController);
}

bool UAOLocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);
	OnControllerChanged(PlayerController);
	
	return bResult;
}

void UAOLocalPlayer::OnControllerChanged(APlayerController* NewController)
{
	
}
