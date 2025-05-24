// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOPlayerController.h"
#include "ModularPlayerState.h"
#include "AOPlayerState.generated.h"

/**
 * 
 */

UCLASS()
class AEGISODYSSEY_API AAOPlayerState : public AModularPlayerState
{
	GENERATED_BODY()
public:
	static const FName NAME_AOAbilityReady;
	UFUNCTION(BlueprintCallable, Category="AOPlayerStateConfig")
	AAOPlayerController* GetAOPlayerController() const;
};
