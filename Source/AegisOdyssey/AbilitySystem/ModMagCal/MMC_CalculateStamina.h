// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_CalculateStamina.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UMMC_CalculateStamina : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_CalculateStamina();
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
	
protected:
	UPROPERTY(EditAnywhere , Category = "Capture")
	FGameplayEffectAttributeCaptureDefinition StaminaDef;
	
	UPROPERTY(EditAnywhere , Category = "Capture")
	FGameplayEffectAttributeCaptureDefinition MaxStaminaDef;
	
	UPROPERTY(EditAnywhere , Category = "Capture")
	FGameplayEffectAttributeCaptureDefinition StaminaBonusDef;
};
