// Fill out your copyright notice in Description page of Project Settings.


#include "MMC_CalculateMaxHealth.h"

#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOPrimaryAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MMC_CalculateMaxHealth)

UMMC_CalculateMaxHealth::UMMC_CalculateMaxHealth()
{
	ConstitutionDef.AttributeToCapture = UAOPrimaryAttributeSet::GetConstitutionAttribute();
	ConstitutionDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	ConstitutionDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(ConstitutionDef);
}

float UMMC_CalculateMaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	// 最大生命值同样只应该吃角色当前真实 Constitution。
	// 这样等级初始化、后续加点和其他 GE 对 Constitution 的修改，
	// 都会统一落到 MaxHealth，而不是被等级表旁路覆盖。
	float Constitution = 0.0f;
	GetCapturedAttributeMagnitude(ConstitutionDef, Spec, EvaluateParameters, Constitution);
	Constitution = FMath::Max(Constitution, 0.0f);

	return 500.0f + Constitution;
}
