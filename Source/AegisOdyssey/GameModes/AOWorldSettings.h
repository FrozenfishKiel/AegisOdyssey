// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOExperienceDefinition.h"
#include "AOExperienceDefinition.h"
#include "GameFramework/WorldSettings.h"
#include "AOWorldSettings.generated.h"

/**
 * 
 */
class UAOExperienceDefinition;
UCLASS()
class AEGISODYSSEY_API AAOWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
public:
	AAOWorldSettings(const FObjectInitializer& ObjectInitializer);

	// Returns the default experience to use when a server opens this map if it is not overridden by the user-facing experience
	FPrimaryAssetId GetDefaultGameplayExperience() const;
	TSubclassOf<UAOExperienceDefinition> GetFeatureDefinitionClass() const;
protected:
	// The default experience to use when a server opens this map if it is not overridden by the user-facing experience
	UPROPERTY(EditDefaultsOnly, Category=GameMode)
	TSoftClassPtr<UAOExperienceDefinition> DefaultGameplayDefinition;
};
