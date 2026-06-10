#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AegisOdyssey/SkillSystem/Execution/AbilityBases/AOSkillGameplayAbility_AreaSequenceBase.h"
#include "AOSkillAreaSequenceRuntime_VolcanoBurst.generated.h"

class UAOSkillGameplayAbility;
class UAOSkillAreaSequenceExecutionDefinition;

struct FTimerHandle;

/**
 * Independent runtime executor for Volcano Burst.
 *
 * Once spawned, it owns the remaining wave sequence lifecycle so the casting ability
 * can end early without blocking movement, roll, or other skills.
 */
UCLASS()
class AEGISODYSSEY_API AAOSkillAreaSequenceRuntime_VolcanoBurst : public AActor
{
	GENERATED_BODY()

public:
	AAOSkillAreaSequenceRuntime_VolcanoBurst(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void InitializeRuntime(
		UAOSkillGameplayAbility* InOwningSkillAbility,
		AActor* InSourceActor,
		UAOSkillAreaSequenceExecutionDefinition* InExecutionDefinition,
		int32 InAbilityLevel);

	void StartRuntimeSequence();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool ComputeNextAreaWave(int32 WaveIndex, FAOSkillAreaWaveResult& OutWave) const;
	void CollectTargetsInImpactRadius(const FVector& ImpactPoint, float ImpactRadius, TArray<AActor*>& OutTargets) const;
	void ExecuteNextWave();
	void FinishRuntime();
	void ClearRuntimeTimers();
	bool CanDrawSkillDebug() const;

private:
	UPROPERTY()
	TWeakObjectPtr<UAOSkillGameplayAbility> OwningSkillAbility;

	UPROPERTY()
	TWeakObjectPtr<AActor> SourceActor;

	UPROPERTY()
	TObjectPtr<UAOSkillAreaSequenceExecutionDefinition> ExecutionDefinition = nullptr;

	UPROPERTY()
	int32 AbilityLevel = 1;

	UPROPERTY()
	int32 ExecutedWaveCount = 0;

	UPROPERTY()
	bool bRuntimeStarted = false;

	FTimerHandle WaveLoopTimerHandle;
};
