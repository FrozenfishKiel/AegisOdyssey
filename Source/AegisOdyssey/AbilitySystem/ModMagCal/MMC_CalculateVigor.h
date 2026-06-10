// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_CalculateVigor.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UMMC_CalculateVigor : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_CalculateVigor();
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
	
protected:
	UPROPERTY(EditAnywhere , Category = "Capture")
	FGameplayEffectAttributeCaptureDefinition VigorBonusDef;
	
	UPROPERTY(EditAnywhere , Category = "Capture")
	FGameplayEffectAttributeCaptureDefinition VigorDef;
		
	UPROPERTY(EditAnywhere , Category = "Capture")
	FGameplayEffectAttributeCaptureDefinition MaxVigorDef;
};
