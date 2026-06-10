#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "AOHarvestGameplayCueNotify_Burst.generated.h"

class UNiagaraSystem;
class USoundBase;
class UAOHarvestableDefinition;

USTRUCT(BlueprintType)
struct FAOHarvestCueBurstEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	TObjectPtr<UNiagaraSystem> NiagaraEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	TObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	FVector Scale = FVector(1.0f);
};

USTRUCT(BlueprintType)
struct FAOHarvestCueVisualSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	TArray<FAOHarvestCueBurstEntry> BurstEntries;
};

// Harvest-specific burst cue.
// Kept separate from the skill cue implementation so harvest presentation stays readable and evolves independently.
UCLASS(Blueprintable)
class AEGISODYSSEY_API UAOHarvestGameplayCueNotify_Burst : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

	static bool PlayVisualSet(const UWorld* World, const FGameplayCueParameters& Parameters, const FAOHarvestCueVisualSet& VisualSet);
	static bool PlayDefinitionVisuals(const UWorld* World, const UAOHarvestableDefinition* HarvestableDefinition,
		const FGameplayCueParameters& Parameters, bool bDepletedAfterHit);

protected:
	// Fallback visuals for generic harvest cues.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	FAOHarvestCueVisualSet DefaultVisualSet;
};
