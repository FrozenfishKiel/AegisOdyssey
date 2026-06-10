#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_CalculateCritChance.generated.h"

UCLASS()
class AEGISODYSSEY_API UMMC_CalculateCritChance : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_CalculateCritChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

protected:
	FGameplayEffectAttributeCaptureDefinition DexterityDef;
	FGameplayEffectAttributeCaptureDefinition WeaponCritChanceDef;
	FGameplayEffectAttributeCaptureDefinition EquipmentCritChanceDef;
};
