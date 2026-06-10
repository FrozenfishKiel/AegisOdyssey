// Fill out your copyright notice in the Description page of Project Settings.

#include "MMC_CalculateDefense.h"

#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Dynamic/AOEquipmentAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOPrimaryAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MMC_CalculateDefense)

UMMC_CalculateDefense::UMMC_CalculateDefense()
{
	ConstitutionDef.AttributeToCapture = UAOPrimaryAttributeSet::GetConstitutionAttribute();
	ConstitutionDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	ConstitutionDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(ConstitutionDef);

	EquipmentDefenseDef.AttributeToCapture = UAOEquipmentAttributeSet::GetEquipmentDefenseAttribute();
	EquipmentDefenseDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EquipmentDefenseDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(EquipmentDefenseDef);
}

float UMMC_CalculateDefense::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Constitution = 0.0f;
	GetCapturedAttributeMagnitude(ConstitutionDef, Spec, EvaluateParameters, Constitution);
	Constitution = FMath::Max(Constitution, 0.0f);

	float EquipmentDefense = 0.0f;
	GetCapturedAttributeMagnitude(EquipmentDefenseDef, Spec, EvaluateParameters, EquipmentDefense);
	EquipmentDefense = FMath::Max(EquipmentDefense, 0.0f);

	// 防御最终值沿用现有派生属性的汇总模式：
	// 角色本体属性提供基础面，正式装备提供额外来源，最终在这里统一相加。
	return Constitution + EquipmentDefense;
}
