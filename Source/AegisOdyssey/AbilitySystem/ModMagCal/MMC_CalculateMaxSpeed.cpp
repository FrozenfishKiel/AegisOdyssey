// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_CalculateMaxSpeed.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/AOCombatAttributeSet.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(MMC_CalculateMaxSpeed)

UMMC_CalculateMaxSpeed::UMMC_CalculateMaxSpeed()
{
	MaxSpeedDef.AttributeToCapture = UAOCombatAttributeSet::GetMaxSpeedAttribute();
	MaxSpeedDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	MaxSpeedDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(MaxSpeedDef);

	SprintSpeedBonusDef.AttributeToCapture = UAOCombatAttributeSet::GetSprintSpeedBonusAttribute();
	SprintSpeedBonusDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	SprintSpeedBonusDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(SprintSpeedBonusDef);

}

float UMMC_CalculateMaxSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluationParameters;
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float MaxSpeed = 0.f;
	GetCapturedAttributeMagnitude(MaxSpeedDef, Spec, EvaluateParameters, MaxSpeed);
	MaxSpeed = FMath::Max<float>(MaxSpeed, 0.f);

	float SprintSpeedBonus = 0.f;
	GetCapturedAttributeMagnitude(SprintSpeedBonusDef, Spec, EvaluateParameters, SprintSpeedBonus);
	SprintSpeedBonus = FMath::Max<float>(SprintSpeedBonus, 0.f);
	
	return MaxSpeed + SprintSpeedBonus;
}
