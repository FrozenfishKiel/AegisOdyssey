#include "AegisOdyssey/Harvest/Cue/AOHarvestGameplayCueNotify_Burst.h"

#include "AegisOdyssey/Harvest/Definition/AOHarvestableDefinition.h"
#include "AegisOdyssey/AOHarvestCueTags.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestGameplayCueNotify_Burst)

bool UAOHarvestGameplayCueNotify_Burst::PlayVisualSet(const UWorld* World, const FGameplayCueParameters& Parameters, const FAOHarvestCueVisualSet& VisualSet)
{
	if (World == nullptr)
	{
		return false;
	}

	bool bPlayedAnyEffect = false;
	for (const FAOHarvestCueBurstEntry& BurstEntry : VisualSet.BurstEntries)
	{
		const FVector SpawnLocation = FVector(Parameters.Location) + BurstEntry.LocationOffset;
		const FRotator SpawnRotation = Parameters.Normal.IsNearlyZero()
			? BurstEntry.RotationOffset
			: Parameters.Normal.Rotation() + BurstEntry.RotationOffset;

		if (BurstEntry.NiagaraEffect != nullptr)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				BurstEntry.NiagaraEffect,
				SpawnLocation,
				SpawnRotation,
				BurstEntry.Scale,
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true);
			bPlayedAnyEffect = true;
		}

		if (BurstEntry.Sound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(World, BurstEntry.Sound, SpawnLocation, SpawnRotation);
			bPlayedAnyEffect = true;
		}
	}

	return bPlayedAnyEffect;
}

bool UAOHarvestGameplayCueNotify_Burst::PlayDefinitionVisuals(const UWorld* World, const UAOHarvestableDefinition* HarvestableDefinition,
	const FGameplayCueParameters& Parameters, bool bDepletedAfterHit)
{
	if (HarvestableDefinition == nullptr)
	{
		return false;
	}

	const FAOHarvestCueVisualSet& VisualSet = bDepletedAfterHit
		? HarvestableDefinition->HarvestDepletedCueVisuals
		: HarvestableDefinition->HarvestHitCueVisuals;
	return PlayVisualSet(World, Parameters, VisualSet);
}

bool UAOHarvestGameplayCueNotify_Burst::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	const UWorld* World = MyTarget != nullptr ? MyTarget->GetWorld() : GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	const UAOHarvestableDefinition* HarvestableDefinition = Cast<UAOHarvestableDefinition>(Parameters.SourceObject.Get());
	if (HarvestableDefinition != nullptr)
	{
		const bool bDepletedAfterHit = Parameters.MatchedTagName == AOHarvestCueTags::GameplayCue_Harvest_Depleted;
		return PlayDefinitionVisuals(World, HarvestableDefinition, Parameters, bDepletedAfterHit);
	}

	return PlayVisualSet(World, Parameters, DefaultVisualSet);
}
