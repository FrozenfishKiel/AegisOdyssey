// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_CalculateVigor.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(MMC_CalculateVigor)

UMMC_CalculateVigor::UMMC_CalculateVigor()
{
	VigorBonusDef.AttributeToCapture = UAOCombatAttributeSet::GetVigorBonusAttribute();
	VigorBonusDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorBonusDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(VigorBonusDef);
	
	VigorDef.AttributeToCapture = UAOCombatAttributeSet::GetVigorAttribute();
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(VigorDef);
	
	MaxVigorDef.AttributeToCapture = UAOCombatAttributeSet::GetMaxVigorAttribute();
	MaxVigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	MaxVigorDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(MaxVigorDef);
}

float UMMC_CalculateVigor::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluationParameters;
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluateParameters, Vigor);
	Vigor = FMath::Max<float>(Vigor, 0.f);
	
	float VigorBonus = 0.0f;
	GetCapturedAttributeMagnitude(VigorBonusDef, Spec, EvaluateParameters, VigorBonus);
	
	float MaxVigor = 0.f;
	GetCapturedAttributeMagnitude(MaxVigorDef, Spec, EvaluateParameters, MaxVigor);
	MaxVigor = FMath::Max<float>(MaxVigor, 0.f);


	return FMath::Clamp(Vigor + VigorBonus,0,MaxVigor);
}
