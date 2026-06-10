// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_CalculateAttack.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UMMC_CalculateAttack : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_CalculateAttack();
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
protected:
	FGameplayEffectAttributeCaptureDefinition StrengthDef;
	FGameplayEffectAttributeCaptureDefinition WeaponAttackDef;
	FGameplayEffectAttributeCaptureDefinition EquipmentAttackDef;
};
