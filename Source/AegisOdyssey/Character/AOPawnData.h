// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AegisOdyssey/Input/AOInputConfig.h"
#include "Engine/DataAsset.h"
#include "AOPawnData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "AO Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class AEGISODYSSEY_API UAOPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "AO|Pawn")
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "AO|Pawn")
	TArray<TObjectPtr<UGameplayAbility>> AbilitySets;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "AO|Pawn")
	TObjectPtr<UGameplayAbility> DefaultAbility;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "AO|Pawn")
	TObjectPtr<UAOInputConfig> InputConfig;
};
