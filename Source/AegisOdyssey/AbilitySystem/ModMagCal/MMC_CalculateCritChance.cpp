#include "MMC_CalculateCritChance.h"

#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Dynamic/AOEquipmentAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOPrimaryAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Dynamic/AOWeaponAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MMC_CalculateCritChance)

UMMC_CalculateCritChance::UMMC_CalculateCritChance()
{
	DexterityDef.AttributeToCapture = UAOPrimaryAttributeSet::GetDexterityAttribute();
	DexterityDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	DexterityDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(DexterityDef);

	WeaponCritChanceDef.AttributeToCapture = UAOWeaponAttributeSet::GetWeaponCritChanceAttribute();
	WeaponCritChanceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	WeaponCritChanceDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(WeaponCritChanceDef);

	EquipmentCritChanceDef.AttributeToCapture = UAOEquipmentAttributeSet::GetEquipmentCritChanceAttribute();
	EquipmentCritChanceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EquipmentCritChanceDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(EquipmentCritChanceDef);
}

float UMMC_CalculateCritChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Dexterity = 0.0f;
	GetCapturedAttributeMagnitude(DexterityDef, Spec, EvaluateParameters, Dexterity);
	Dexterity = FMath::Max(Dexterity, 0.0f);

	float WeaponCritChance = 0.0f;
	GetCapturedAttributeMagnitude(WeaponCritChanceDef, Spec, EvaluateParameters, WeaponCritChance);
	WeaponCritChance = FMath::Max(WeaponCritChance, 0.0f);

	float EquipmentCritChance = 0.0f;
	GetCapturedAttributeMagnitude(EquipmentCritChanceDef, Spec, EvaluateParameters, EquipmentCritChance);
	EquipmentCritChance = FMath::Max(EquipmentCritChance, 0.0f);

	return Dexterity + WeaponCritChance + EquipmentCritChance;
}
