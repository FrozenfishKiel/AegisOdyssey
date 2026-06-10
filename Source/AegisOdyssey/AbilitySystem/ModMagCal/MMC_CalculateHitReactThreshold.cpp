// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_CalculateHitReactThreshold.h"

#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOPrimaryAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MMC_CalculateHitReactThreshold)

UMMC_CalculateHitReactThreshold::UMMC_CalculateHitReactThreshold()
{
	HitReactThresholdDef.AttributeToCapture = UAOPrimaryAttributeSet::GetHitReactThresholdAttribute();
	HitReactThresholdDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	HitReactThresholdDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(HitReactThresholdDef);
}

float UMMC_CalculateHitReactThreshold::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float HitReactThreshold = 0.0f;
	GetCapturedAttributeMagnitude(HitReactThresholdDef, Spec, EvaluateParameters, HitReactThreshold);
	return FMath::Max(HitReactThreshold, 0.0f);
}
