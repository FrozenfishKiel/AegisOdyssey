// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_CalculateResistance.generated.h"

UCLASS()
class AEGISODYSSEY_API UMMC_CalculateResistance : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_CalculateResistance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

protected:
	FGameplayEffectAttributeCaptureDefinition IntelligenceDef;
	FGameplayEffectAttributeCaptureDefinition EquipmentResistanceDef;
};
