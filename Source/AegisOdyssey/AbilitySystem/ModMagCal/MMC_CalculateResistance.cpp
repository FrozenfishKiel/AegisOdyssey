// Fill out your copyright notice in the Description page of Project Settings.

#include "MMC_CalculateResistance.h"

#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Dynamic/AOEquipmentAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOPrimaryAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MMC_CalculateResistance)

UMMC_CalculateResistance::UMMC_CalculateResistance()
{
	IntelligenceDef.AttributeToCapture = UAOPrimaryAttributeSet::GetIntelligenceAttribute();
	IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntelligenceDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(IntelligenceDef);

	EquipmentResistanceDef.AttributeToCapture = UAOEquipmentAttributeSet::GetEquipmentResistanceAttribute();
	EquipmentResistanceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EquipmentResistanceDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(EquipmentResistanceDef);
}

float UMMC_CalculateResistance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Intelligence = 0.0f;
	GetCapturedAttributeMagnitude(IntelligenceDef, Spec, EvaluateParameters, Intelligence);
	Intelligence = FMath::Max(Intelligence, 0.0f);

	float EquipmentResistance = 0.0f;
	GetCapturedAttributeMagnitude(EquipmentResistanceDef, Spec, EvaluateParameters, EquipmentResistance);
	EquipmentResistance = FMath::Max(EquipmentResistance, 0.0f);

	// 当前先做“总抗性”：角色本体提供基础抗性，正式装备提供额外抗性，
	// 在最终属性层统一汇总，避免把来源拆成多个无意义的子属性。
	return Intelligence + EquipmentResistance;
}
