// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_CalculateStamina.h"

#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MMC_CalculateStamina)

UMMC_CalculateStamina::UMMC_CalculateStamina()
{
	StaminaDef.AttributeToCapture = UAOCombatAttributeSet::GetStaminaAttribute();
	StaminaDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	StaminaDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(StaminaDef);
	
	MaxStaminaDef.AttributeToCapture = UAOCombatAttributeSet::GetMaxStaminaAttribute();
	MaxStaminaDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	MaxStaminaDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(MaxStaminaDef);
	
	StaminaBonusDef.AttributeToCapture = UAOCombatAttributeSet::GetStaminaBonusAttribute();
	StaminaBonusDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	StaminaBonusDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(StaminaBonusDef);
}

float UMMC_CalculateStamina::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluationParameters;
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	float MaxStamina = 0.f;
	GetCapturedAttributeMagnitude(MaxStaminaDef , Spec , EvaluationParameters , MaxStamina);
	MaxStamina = FMath::Max(MaxStamina , 0.f);
	
	float Stamina = 0.f;
	GetCapturedAttributeMagnitude(StaminaDef , Spec , EvaluationParameters , Stamina);
	
	float StaminaBonus = 0.f;
	GetCapturedAttributeMagnitude(StaminaBonusDef , Spec , EvaluationParameters , StaminaBonus);
	
	return FMath::Clamp(Stamina + StaminaBonus , 0.f , MaxStamina);
	
}
