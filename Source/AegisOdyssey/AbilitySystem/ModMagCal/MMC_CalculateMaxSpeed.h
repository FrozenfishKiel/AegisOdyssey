// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_CalculateMaxSpeed.generated.h"

class UAOCombatAttributeSet;

UCLASS()
class AEGISODYSSEY_API UMMC_CalculateMaxSpeed : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_CalculateMaxSpeed();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Capture")
	FGameplayEffectAttributeCaptureDefinition MaxSpeedDef;

	UPROPERTY(EditAnywhere, Category = "Capture")
	FGameplayEffectAttributeCaptureDefinition SprintSpeedBonusDef;
};
