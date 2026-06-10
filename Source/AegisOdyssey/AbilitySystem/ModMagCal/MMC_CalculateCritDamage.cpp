#include "MMC_CalculateCritDamage.h"

#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Dynamic/AOWeaponAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MMC_CalculateCritDamage)

UMMC_CalculateCritDamage::UMMC_CalculateCritDamage()
{
	WeaponCritDamageDef.AttributeToCapture = UAOWeaponAttributeSet::GetWeaponCritDamageAttribute();
	WeaponCritDamageDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	WeaponCritDamageDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(WeaponCritDamageDef);
}

float UMMC_CalculateCritDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float WeaponCritDamage = 0.0f;
	GetCapturedAttributeMagnitude(WeaponCritDamageDef, Spec, EvaluateParameters, WeaponCritDamage);
	return FMath::Max(WeaponCritDamage, 0.0f);
}
