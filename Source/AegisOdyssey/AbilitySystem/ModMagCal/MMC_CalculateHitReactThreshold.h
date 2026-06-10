#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_CalculateHitReactThreshold.generated.h"

UCLASS()
class AEGISODYSSEY_API UMMC_CalculateHitReactThreshold : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_CalculateHitReactThreshold();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition HitReactThresholdDef;
};
