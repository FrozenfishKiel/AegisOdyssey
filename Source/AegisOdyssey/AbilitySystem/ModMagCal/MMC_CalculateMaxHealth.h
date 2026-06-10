// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_CalculateMaxHealth.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UMMC_CalculateMaxHealth : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_CalculateMaxHealth();
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
protected:
	FGameplayEffectAttributeCaptureDefinition ConstitutionDef;
};
