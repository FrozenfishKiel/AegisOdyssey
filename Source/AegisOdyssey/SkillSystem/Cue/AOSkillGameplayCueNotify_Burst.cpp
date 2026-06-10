#include "AegisOdyssey/SkillSystem/Cue/AOSkillGameplayCueNotify_Burst.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillGameplayCueNotify_Burst)

bool UAOSkillGameplayCueNotify_Burst::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	const UWorld* World = MyTarget != nullptr ? MyTarget->GetWorld() : GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	bool bPlayedAnyEffect = false;
	for (const FAOSkillCueBurstEntry& BurstEntry : BurstEntries)
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
