// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_CalculateAttack.h"

#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOPrimaryAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Dynamic/AOEquipmentAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Dynamic/AOWeaponAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MMC_CalculateAttack)

UMMC_CalculateAttack::UMMC_CalculateAttack()
{
	StrengthDef.AttributeToCapture = UAOPrimaryAttributeSet::GetStrengthAttribute();
	StrengthDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	StrengthDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(StrengthDef);

	WeaponAttackDef.AttributeToCapture = UAOWeaponAttributeSet::GetWeaponAttackAttribute();
	WeaponAttackDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	WeaponAttackDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(WeaponAttackDef);

	EquipmentAttackDef.AttributeToCapture = UAOEquipmentAttributeSet::GetEquipmentAttackAttribute();
	EquipmentAttackDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EquipmentAttackDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(EquipmentAttackDef);
}

float UMMC_CalculateAttack::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	// 攻击力必须读取角色当前真实 Strength。
	// 这样角色升级初始化、手动加点、以及其他 GE 对 Strength 的修改，
	// 都会统一反映到最终攻击力，而不会和等级表里的理论值脱节。
	float Strength = 0.0f;
	GetCapturedAttributeMagnitude(StrengthDef, Spec, EvaluateParameters, Strength);
	Strength = FMath::Max(Strength, 0.0f);

	float WeaponAttack = 0.0f;
	GetCapturedAttributeMagnitude(WeaponAttackDef, Spec, EvaluateParameters, WeaponAttack);
	WeaponAttack = FMath::Max(WeaponAttack, 0.0f);

	float EquipmentAttack = 0.0f;
	GetCapturedAttributeMagnitude(EquipmentAttackDef, Spec, EvaluateParameters, EquipmentAttack);
	EquipmentAttack = FMath::Max(EquipmentAttack, 0.0f);

	// 当前攻击力语义：
	// 角色当前 Strength + 武器来源攻击力 + 正式装备来源攻击力。
	return Strength + WeaponAttack + EquipmentAttack;
}
