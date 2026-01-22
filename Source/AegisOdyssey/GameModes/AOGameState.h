// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOExperienceManagerComponent.h"
#include "ModularGameState.h"
#include "AegisOdyssey/Player/AOPlayerSpawningManagerComponent.h"
#include "AOGameState.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API AAOGameState : public AModularGameStateBase
{
	GENERATED_BODY()
public:
	AAOGameState();
private:
	UPROPERTY()
	TObjectPtr<UAOExperienceManagerComponent> AOGameStateComponent;
	UPROPERTY()
	TObjectPtr<UAOPlayerSpawningManagerComponent> APlayerSpawningManagerComponent;
};
