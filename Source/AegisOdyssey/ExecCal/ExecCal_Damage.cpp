#include "ExecCal_Damage.h"

#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AOAbilityTypes.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/AOPublicFunctionLibrary.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOCharacterCombatManagerComponent.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExecCal_Damage)

struct AODamageStatic
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Attack);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MaxHealth);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);

	AODamageStatic()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOCombatAttributeSet, Attack, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOCombatAttributeSet, CritChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOCombatAttributeSet, CritDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOHealthAttributeSet, Health, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAOHealthAttributeSet, MaxHealth, Target, false);
	}
};

static const AODamageStatic& GetDamageStatic()
{
	static AODamageStatic DamageStatic;
	return DamageStatic;
}

UExecCal_Damage::UExecCal_Damage()
{
	RelevantAttributesToCapture.Add(GetDamageStatic().AttackDef);
	RelevantAttributesToCapture.Add(GetDamageStatic().CritChanceDef);
	RelevantAttributesToCapture.Add(GetDamageStatic().CritDamageDef);
	RelevantAttributesToCapture.Add(GetDamageStatic().HealthDef);
	RelevantAttributesToCapture.Add(GetDamageStatic().MaxHealthDef);
}

void UExecCal_Damage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	if (SourceASC == nullptr || TargetASC == nullptr)
	{
		return;
	}

	AActor* SourceAvatar = SourceASC->GetAvatarActor();
	AActor* TargetAvatar = TargetASC->GetAvatarActor();
	if (SourceAvatar == nullptr || TargetAvatar == nullptr)
	{
		return;
	}

	const UAOExtPawnComponent* SourceExtPawn = SourceAvatar->GetComponentByClass<UAOExtPawnComponent>();
	const UAOExtPawnComponent* TargetExtPawn = TargetAvatar->GetComponentByClass<UAOExtPawnComponent>();
	if (SourceExtPawn == nullptr || TargetExtPawn == nullptr)
	{
		return;
	}

	(void)SourceExtPawn->GetPawnData<UAOPawnData>();
	(void)TargetExtPawn->GetPawnData<UAOPawnData>();

	FAOGameplayEffectContext* SourceEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContextHandle.Get());
	check(SourceEffectContext);

	const bool bWasBlocked = SourceEffectContext->GetWasBlocked();
	const bool bWasParried = SourceEffectContext->GetWasParried();
	const bool bHitInvulnerability = SourceEffectContext->GetHitInvulnerability();

	float DamageMultiplier = 1.0f;
	float RequestedHitStaminaDamage = 0.0f;
	bool bCanDealHealthDamage = !bWasParried && !bHitInvulnerability;

	if (AAOCharacter* TargetCharacter = Cast<AAOCharacter>(TargetAvatar))
	{
		if (UAOCharacterCombatManagerComponent* CombatManager = TargetCharacter->FindComponentByClass<UAOCharacterCombatManagerComponent>())
		{
			if (bWasBlocked)
			{
				RequestedHitStaminaDamage = CombatManager->GetBlockStaminaDamage();
				DamageMultiplier = TargetASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Block.Blocking")))
					? 0.0f
					: CombatManager->GetPartialBlockDamageMultiplier();
			}
			else
			{
				RequestedHitStaminaDamage = CombatManager->GetHitStaminaDamage();
			}
		}
	}

	if (bWasBlocked && !TargetASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Block.Blocking"))))
	{
		DamageMultiplier = 0.35f;
		if (AAOCharacter* TargetCharacter = Cast<AAOCharacter>(TargetAvatar))
		{
			if (UAOCharacterCombatManagerComponent* CombatManager = TargetCharacter->FindComponentByClass<UAOCharacterCombatManagerComponent>())
			{
				DamageMultiplier = CombatManager->GetPartialBlockDamageMultiplier();
			}
		}
	}

	if (!bCanDealHealthDamage)
	{
		DamageMultiplier = 0.0f;
		RequestedHitStaminaDamage = 0.0f;
	}
	else if (!bWasBlocked && RequestedHitStaminaDamage <= 0.0f)
	{
		RequestedHitStaminaDamage = 0.0f;
	}

	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][ExecCal] Enter. Source=%s Target=%s AttackTag=%s SkillTag=%s WeaponTag=%s bWasBlocked=%s bWasParried=%s bHitInvulnerability=%s DamageMultiplier=%.2f RequestedHitStaminaDamage=%.2f bCanDealHealthDamage=%s"),
		*GetNameSafe(SourceAvatar),
		*GetNameSafe(TargetAvatar),
		*SourceEffectContext->GetAttackTag().ToString(),
		*SourceEffectContext->GetSkillTag().ToString(),
		*SourceEffectContext->GetWeaponTag().ToString(),
		bWasBlocked ? TEXT("true") : TEXT("false"),
		bWasParried ? TEXT("true") : TEXT("false"),
		bHitInvulnerability ? TEXT("true") : TEXT("false"),
		DamageMultiplier,
		RequestedHitStaminaDamage,
		bCanDealHealthDamage ? TEXT("true") : TEXT("false"));

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParams;
	EvaluateParams.SourceTags = SourceTags;
	EvaluateParams.TargetTags = TargetTags;

	float SourceAttackPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetDamageStatic().AttackDef,
		EvaluateParams,
		SourceAttackPower);

	float SourceCritChance = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetDamageStatic().CritChanceDef,
		EvaluateParams,
		SourceCritChance);

	float SourceCritDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetDamageStatic().CritDamageDef,
		EvaluateParams,
		SourceCritDamage);

	bool bIsCritical = false;
	if (bCanDealHealthDamage)
	{
		const float RandomValue = FMath::FRand();
		if (RandomValue <= SourceCritChance)
		{
			bIsCritical = true;
			SourceEffectContext->SetIsCritical(true);
		}
	}

	float SourceHealth = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetDamageStatic().HealthDef,
		EvaluateParams,
		SourceHealth);

	float FinalDamageDone = SourceAttackPower;
	if (bIsCritical)
	{
		FinalDamageDone *= (1.0f + SourceCritDamage);
	}
	FinalDamageDone *= DamageMultiplier;

	float ResolvedHitStaminaDamage = 0.0f;
	if (RequestedHitStaminaDamage > 0.0f)
	{
		ResolvedHitStaminaDamage = RequestedHitStaminaDamage;

		const float OldStamina = TargetASC->GetNumericAttribute(UAOCombatAttributeSet::GetStaminaAttribute());
		TargetASC->ApplyModToAttribute(
			UAOCombatAttributeSet::GetStaminaAttribute(),
			EGameplayModOp::Additive,
			-ResolvedHitStaminaDamage);
		const float NewStamina = TargetASC->GetNumericAttribute(UAOCombatAttributeSet::GetStaminaAttribute());

		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][ExecCal] Apply stamina damage. Target=%s Requested=%.2f OldStamina=%.2f NewStamina=%.2f"),
			*GetNameSafe(TargetAvatar),
			ResolvedHitStaminaDamage,
			OldStamina,
			NewStamina);

	}

	SourceEffectContext->SetResolvedHitStaminaDamage(ResolvedHitStaminaDamage);
	const float ResolvedHitReactStrength = FinalDamageDone + ResolvedHitStaminaDamage * 3.0f;
	SourceEffectContext->SetResolvedHitReactStrength(ResolvedHitReactStrength);

	UE_LOG(
		LogAegisOdysseyAbilitySystem,
		Warning,
		TEXT("UExecCal_Damage::Execute_Implementation: AttackTag=%s SkillTag=%s WeaponTag=%s SourceAttackPower=%.2f, SourceCritChance=%.2f, SourceCritDamage=%.2f, bIsCritical=%s, FinalDamageDone=%.2f, HitStaminaDamage=%.2f, HitReactStrength=%.2f, Health=%.2f"),
		*SourceEffectContext->GetAttackTag().ToString(),
		*SourceEffectContext->GetSkillTag().ToString(),
		*SourceEffectContext->GetWeaponTag().ToString(),
		SourceAttackPower,
		SourceCritChance,
		SourceCritDamage,
		bIsCritical ? TEXT("true") : TEXT("false"),
		FinalDamageDone,
		ResolvedHitStaminaDamage,
		ResolvedHitReactStrength,
		SourceHealth);

	if (FinalDamageDone > 0.0f)
	{
		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][ExecCal] Output damage. Target=%s FinalDamageDone=%.2f"),
			*GetNameSafe(TargetAvatar),
			FinalDamageDone);

		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UAOHealthAttributeSet::GetDamageAttribute(),
			EGameplayModOp::Override,
			FinalDamageDone));
	}
	else
	{
		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][ExecCal] No damage output. Target=%s FinalDamageDone=%.2f"),
			*GetNameSafe(TargetAvatar),
			FinalDamageDone);
	}
}

