#include "AegisOdyssey/SkillSystem/Execution/Runtime/AOSkillAreaSequenceRuntime_VolcanoBurst.h"

#include "AegisOdyssey/Character/CombatInterface.h"
#include "AegisOdyssey/SkillSystem/Core/AOSkillGameplayAbility.h"
#include "AegisOdyssey/SkillSystem/Execution/Definitions/AOSkillExecutionDefinition_AreaSequence.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillAreaSequenceRuntime_VolcanoBurst)

namespace AOSkillAreaSequenceRuntimeVolcanoBurstPrivate
{
	static TAutoConsoleVariable<int32> CVarSkillDebugDraw(
		TEXT("ao.Skill.DebugDraw"),
		0,
		TEXT("Enable debug draw for AO skill execution helpers."));
}

AAOSkillAreaSequenceRuntime_VolcanoBurst::AAOSkillAreaSequenceRuntime_VolcanoBurst(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
}

void AAOSkillAreaSequenceRuntime_VolcanoBurst::InitializeRuntime(
	UAOSkillGameplayAbility* InOwningSkillAbility,
	AActor* InSourceActor,
	UAOSkillAreaSequenceExecutionDefinition* InExecutionDefinition,
	int32 InAbilityLevel)
{
	OwningSkillAbility = InOwningSkillAbility;
	SourceActor = InSourceActor;
	ExecutionDefinition = InExecutionDefinition;
	AbilityLevel = FMath::Max(1, InAbilityLevel);
	ExecutedWaveCount = 0;
	bRuntimeStarted = false;

	if (InSourceActor != nullptr)
	{
		SetActorLocation(InSourceActor->GetActorLocation());
		SetActorRotation(InSourceActor->GetActorRotation());
	}
}

void AAOSkillAreaSequenceRuntime_VolcanoBurst::StartRuntimeSequence()
{
	if (bRuntimeStarted)
	{
		return;
	}

	bRuntimeStarted = true;

	if (ExecutionDefinition == nullptr || !SourceActor.IsValid() || !OwningSkillAbility.IsValid())
	{
		FinishRuntime();
		return;
	}

	ExecuteNextWave();

	if (ExecutionDefinition == nullptr || ExecutedWaveCount >= ExecutionDefinition->WaveCount)
	{
		FinishRuntime();
		return;
	}

	const float WaveInterval = FMath::Max(0.0f, ExecutionDefinition->WaveInterval.GetValueAtLevel(AbilityLevel));
	if (WaveInterval <= 0.0f)
	{
		while (ExecutionDefinition != nullptr && ExecutedWaveCount < ExecutionDefinition->WaveCount)
		{
			ExecuteNextWave();
		}

		FinishRuntime();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			WaveLoopTimerHandle,
			this,
			&ThisClass::ExecuteNextWave,
			WaveInterval,
			true);
	}
	else
	{
		FinishRuntime();
	}
}

void AAOSkillAreaSequenceRuntime_VolcanoBurst::BeginPlay()
{
	Super::BeginPlay();
}

void AAOSkillAreaSequenceRuntime_VolcanoBurst::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearRuntimeTimers();
	Super::EndPlay(EndPlayReason);
}

bool AAOSkillAreaSequenceRuntime_VolcanoBurst::ComputeNextAreaWave(int32 WaveIndex, FAOSkillAreaWaveResult& OutWave) const
{
	if (ExecutionDefinition == nullptr || !SourceActor.IsValid())
	{
		return false;
	}

	if (WaveIndex < 0 || WaveIndex >= ExecutionDefinition->WaveCount)
	{
		return false;
	}

	const FVector Forward = SourceActor->GetActorForwardVector();
	const FVector Right = SourceActor->GetActorRightVector();
	const FVector Center =
		SourceActor->GetActorLocation()
		+ Forward * ExecutionDefinition->AreaCenterForwardDistance.GetValueAtLevel(AbilityLevel)
		+ SourceActor->GetActorRotation().RotateVector(ExecutionDefinition->AreaCenterOffset);

	const float AreaRadius = ExecutionDefinition->AreaRadius.GetValueAtLevel(AbilityLevel);
	const float ImpactRadius = ExecutionDefinition->ImpactRadius.GetValueAtLevel(AbilityLevel);
	const FVector2D RandomInCircle = FMath::RandPointInCircle(AreaRadius);
	const FVector ImpactPoint = Center + Forward * RandomInCircle.X + Right * RandomInCircle.Y;

	OutWave.AreaCenter = Center;
	OutWave.ImpactPoint = ImpactPoint;
	OutWave.AreaRadius = AreaRadius;
	OutWave.ImpactRadius = ImpactRadius;

	if (CanDrawSkillDebug())
	{
		const FAOSkillDebugConfig& DebugConfig = ExecutionDefinition->DebugConfig;
		DrawDebugSphere(GetWorld(), Center, AreaRadius, 24, DebugConfig.PrimaryDebugColor.ToFColor(true), false, DebugConfig.DebugDrawDuration);
		DrawDebugSphere(GetWorld(), ImpactPoint, ImpactRadius, 16, DebugConfig.SecondaryDebugColor.ToFColor(true), false, DebugConfig.DebugDrawDuration);
	}

	return true;
}

void AAOSkillAreaSequenceRuntime_VolcanoBurst::CollectTargetsInImpactRadius(
	const FVector& ImpactPoint,
	float ImpactRadius,
	TArray<AActor*>& OutTargets) const
{
	OutTargets.Reset();

	if (!GetWorld() || ImpactRadius <= 0.0f)
	{
		return;
	}

	TArray<AActor*> IgnoreActors;
	if (SourceActor.IsValid())
	{
		IgnoreActors.Add(SourceActor.Get());
	}

	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);

	if (!UKismetSystemLibrary::SphereOverlapActors(
		this,
		ImpactPoint,
		ImpactRadius,
		ObjectTypes,
		AActor::StaticClass(),
		IgnoreActors,
		OverlappedActors))
	{
		return;
	}

	for (AActor* Target : OverlappedActors)
	{
		if (Target && Target->Implements<UCombatInterface>())
		{
			OutTargets.AddUnique(Target);
		}
	}
}

void AAOSkillAreaSequenceRuntime_VolcanoBurst::ExecuteNextWave()
{
	if (ExecutionDefinition == nullptr || !SourceActor.IsValid() || !OwningSkillAbility.IsValid())
	{
		FinishRuntime();
		return;
	}

	if (ExecutedWaveCount >= ExecutionDefinition->WaveCount)
	{
		FinishRuntime();
		return;
	}

	FAOSkillAreaWaveResult WaveResult;
	if (!ComputeNextAreaWave(ExecutedWaveCount, WaveResult))
	{
		FinishRuntime();
		return;
	}

	if (ExecutionDefinition->CueConfig.ExecuteCueTag.IsValid())
	{
		const FGameplayCueParameters CueParameters =
			OwningSkillAbility->BuildSkillCueParameters(WaveResult.ImpactPoint, FVector::UpVector, this);
		OwningSkillAbility->ExecuteSkillCue(ExecutionDefinition->CueConfig.ExecuteCueTag, CueParameters);
	}

	TArray<AActor*> HitTargets;
	CollectTargetsInImpactRadius(WaveResult.ImpactPoint, WaveResult.ImpactRadius, HitTargets);

	FHitResult WaveHitResult;
	WaveHitResult.ImpactPoint = WaveResult.ImpactPoint;
	WaveHitResult.Location = WaveResult.ImpactPoint;
	WaveHitResult.ImpactNormal = FVector::UpVector;
	WaveHitResult.Normal = FVector::UpVector;
	WaveHitResult.TraceStart = WaveResult.AreaCenter;
	WaveHitResult.TraceEnd = WaveResult.ImpactPoint;

	FName SegmentKey = NAME_None;
	if (ExecutionDefinition->EffectConfig.HitPolicy.PolicyType == EAOCombatHitPolicyType::SegmentedHit)
	{
		SegmentKey = OwningSkillAbility->BuildRuntimeHitSegmentKey(NAME_None, ExecutedWaveCount);
	}
	OwningSkillAbility->RouteSkillEffectApplicationFromRuntimeActor(HitTargets, &WaveHitResult, this, SegmentKey);

	++ExecutedWaveCount;

	if (ExecutedWaveCount >= ExecutionDefinition->WaveCount)
	{
		FinishRuntime();
	}
}

void AAOSkillAreaSequenceRuntime_VolcanoBurst::FinishRuntime()
{
	ClearRuntimeTimers();
	Destroy();
}

void AAOSkillAreaSequenceRuntime_VolcanoBurst::ClearRuntimeTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WaveLoopTimerHandle);
	}
}

bool AAOSkillAreaSequenceRuntime_VolcanoBurst::CanDrawSkillDebug() const
{
	return ExecutionDefinition != nullptr
		&& ExecutionDefinition->DebugConfig.bEnableDebugDraw
		&& AOSkillAreaSequenceRuntimeVolcanoBurstPrivate::CVarSkillDebugDraw.GetValueOnGameThread() != 0;
}
