#include "AegisOdyssey/Combat/Cue/AOCombatGameplayCueNotify_Burst.h"

#include "AegisOdyssey/AOAbilityTypes.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "Sound/SoundBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatGameplayCueNotify_Burst)

bool UAOCombatGameplayCueNotify_Burst::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	UE_LOG(
	LogAegisOdysseyCombatTrace,
	Error,
	TEXT("进入GC！！！"));
	const UWorld* World = MyTarget != nullptr ? MyTarget->GetWorld() : GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	bool bPlayedAnyEffect = false;
	for (const FAOCombatCueBurstEffectGroup& EffectGroup : EffectGroups)
	{
		bPlayedAnyEffect |= PlayEffectGroup(World, Parameters, EffectGroup);
		if (bPlayedAnyEffect && bStopAfterFirstMatchingGroup)
		{
			break;
		}
	}

	return bPlayedAnyEffect;
}

bool UAOCombatGameplayCueNotify_Burst::PlayEffectGroup(const UWorld* World, const FGameplayCueParameters& Parameters, const FAOCombatCueBurstEffectGroup& EffectGroup)
{
	if (World == nullptr)
	{
		return false;
	}

	bool bPlayedAnyEffect = false;
	for (const FAOCombatCueBurstEffectEntry& EffectEntry : EffectGroup.EffectEntries)
	{
		const FVector SpawnLocation = FVector(Parameters.Location) + EffectEntry.LocationOffset;
		const FRotator SpawnRotation = Parameters.Normal.IsNearlyZero()
			? EffectEntry.RotationOffset
			: Parameters.Normal.Rotation() + EffectEntry.RotationOffset;

		if (EffectEntry.NiagaraEffect != nullptr)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				EffectEntry.NiagaraEffect,
				SpawnLocation,
				SpawnRotation,
				EffectEntry.Scale,
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true);
			bPlayedAnyEffect = true;
				UE_LOG(
				LogAegisOdysseyCombatTrace,
				Error,
				TEXT("生成效果！！！ "));
		}

		if (EffectEntry.Sound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(World, EffectEntry.Sound, SpawnLocation, SpawnRotation);
			bPlayedAnyEffect = true;
		}
	}

	return bPlayedAnyEffect;
}

bool UAOCombatGameplayCueNotify_Burst::MatchesEffectGroup(const FAOCombatCueBurstEffectGroup& EffectGroup, const FGameplayCueParameters& Parameters)
{
	const FAOGameplayEffectContext* CombatEffectContext =
		Parameters.EffectContext.IsValid()
			? static_cast<const FAOGameplayEffectContext*>(Parameters.EffectContext.Get())
			: nullptr;

	if (!MatchesRequiredTags(EffectGroup.RequiredSourceTags, Parameters.AggregatedSourceTags))
	{
		UE_LOG(LogAegisOdysseyCombatTrace, Error, TEXT("GC失败: SourceTags"));
		return false;
	}

	if (!MatchesRequiredTags(EffectGroup.RequiredTargetTags, Parameters.AggregatedTargetTags))
	{
		UE_LOG(LogAegisOdysseyCombatTrace, Error, TEXT("GC失败: TargetTags"));
		return false;
	}

	if (!EffectGroup.RequiredAttackTags.IsEmpty())
	{
		if (CombatEffectContext == nullptr || !EffectGroup.RequiredAttackTags.HasTagExact(CombatEffectContext->GetAttackTag()))
		{
			UE_LOG(LogAegisOdysseyCombatTrace, Error, TEXT("GC失败: AttackTag"));
			return false;
		}
	}

	if (!EffectGroup.RequiredSkillTags.IsEmpty())
	{
		if (CombatEffectContext == nullptr || !EffectGroup.RequiredSkillTags.HasTagExact(CombatEffectContext->GetSkillTag()))
		{
			return false;
		}
	}

	if (!EffectGroup.RequiredWeaponTags.IsEmpty())
	{
		if (CombatEffectContext == nullptr || !EffectGroup.RequiredWeaponTags.HasTagExact(CombatEffectContext->GetWeaponTag()))
		{
			return false;
		}
	}

	if (!EffectGroup.RequiredDamageTypeTags.IsEmpty())
	{
		if (CombatEffectContext == nullptr || !CombatEffectContext->GetDamageTypeTags().HasAllExact(EffectGroup.RequiredDamageTypeTags))
		{
			return false;
		}
	}

	if (!MatchesBoolRequirement(
		EffectGroup.CriticalRequirement,
		CombatEffectContext != nullptr && CombatEffectContext->GetIsCritical()))
	{
		return false;
	}

	if (!MatchesBoolRequirement(
		EffectGroup.BlockedRequirement,
		CombatEffectContext != nullptr && CombatEffectContext->GetWasBlocked()))
	{
		return false;
	}

	if (!MatchesBoolRequirement(
		EffectGroup.ParriedRequirement,
		CombatEffectContext != nullptr && CombatEffectContext->GetWasParried()))
	{
		return false;
	}

	if (!MatchesBoolRequirement(
		EffectGroup.HitInvulnerabilityRequirement,
		CombatEffectContext != nullptr && CombatEffectContext->GetHitInvulnerability()))
	{
		return false;
	}

	return true;
}

bool UAOCombatGameplayCueNotify_Burst::MatchesBoolRequirement(const EAOCombatCueBoolRequirement Requirement, const bool bActualValue)
{
	switch (Requirement)
	{
	case EAOCombatCueBoolRequirement::Ignore:
		return true;

	case EAOCombatCueBoolRequirement::MustBeTrue:
		return bActualValue;

	case EAOCombatCueBoolRequirement::MustBeFalse:
		return !bActualValue;

	default:
		return true;
	}
}

bool UAOCombatGameplayCueNotify_Burst::MatchesRequiredTags(const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& ActualTags)
{
	return RequiredTags.IsEmpty() || ActualTags.HasAllExact(RequiredTags);
}
