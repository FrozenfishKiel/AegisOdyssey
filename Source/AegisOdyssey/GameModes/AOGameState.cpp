// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameState.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOGameState)

AAOGameState::AAOGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AOGameStateComponent = CreateDefaultSubobject<UAOExperienceManagerComponent>(TEXT("GameStateComponnet"));
	APlayerSpawningManagerComponent = CreateDefaultSubobject<UAOPlayerSpawningManagerComponent>(TEXT("PlayerSpawningManagerComponent"));
}
