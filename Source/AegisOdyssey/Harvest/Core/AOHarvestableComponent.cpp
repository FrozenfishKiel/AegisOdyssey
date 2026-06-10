#include "AegisOdyssey/Harvest/Core/AOHarvestableComponent.h"

#include "AegisOdyssey/Harvest/Cue/AOHarvestGameplayCueNotify_Burst.h"
#include "AegisOdyssey/Harvest/Definition/AOHarvestableDefinition.h"
#include "AegisOdyssey/Harvest/Core/AOHarvestableTarget.h"
#include "GameplayEffectTypes.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestableComponent)

UAOHarvestableComponent::UAOHarvestableComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UAOHarvestableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		RefreshRuntimeStateFromDefinition();
	}
}

void UAOHarvestableComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, LastDepletedEvent);
	DOREPLIFETIME(ThisClass, RuntimeState);
}

float UAOHarvestableComponent::GetTotalHarvestProgress() const
{
	return HarvestableDefinition ? HarvestableDefinition->TotalHarvestProgress : 0.0f;
}

bool UAOHarvestableComponent::CanAcceptHarvestRequest() const
{
	return HarvestableDefinition != nullptr &&
		!RuntimeState.bDepleted &&
		!RuntimeState.bRespawnPending &&
		RuntimeState.CurrentProgress > 0.0f;
}

bool UAOHarvestableComponent::ApplyHarvestResult(const FAOHarvestResult& HarvestResult)
{
	FAOHarvestLifecycleContext LifecycleContext;
	LifecycleContext.HarvestResult = HarvestResult;
	return ApplyHarvestResultWithContext(LifecycleContext);
}

bool UAOHarvestableComponent::ApplyHarvestResultWithContext(const FAOHarvestLifecycleContext& LifecycleContext)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !LifecycleContext.HarvestResult.bSuccess)
	{
		return false;
	}

	const bool bWasDepletedBeforeApply = RuntimeState.bDepleted;

	RuntimeState.CurrentProgress = FMath::Max(0.0f, LifecycleContext.HarvestResult.RemainingProgress);
	RuntimeState.bDepleted = LifecycleContext.HarvestResult.bDepletedAfterHit || RuntimeState.CurrentProgress <= 0.0f;

	if (!bWasDepletedBeforeApply && RuntimeState.bDepleted)
	{
		UpdateReplicatedDepletedEvent(LifecycleContext);
		BroadcastNodeDepleted(LifecycleContext);
		StartRespawnTimerIfNeeded();
	}

	return true;
}

bool UAOHarvestableComponent::ResolveHarvestProgressRequest(float RequestedProgress, FAOHarvestResult& InOutResult) const
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !CanAcceptHarvestRequest())
	{
		return false;
	}

	const float PreviousProgress = RuntimeState.CurrentProgress;
	const float AppliedProgress = FMath::Clamp(RequestedProgress, 0.0f, PreviousProgress);
	const float RemainingProgress = FMath::Max(0.0f, PreviousProgress - AppliedProgress);
	const bool bWillDeplete = RemainingProgress <= KINDA_SMALL_NUMBER;

	InOutResult.PreviousProgress = PreviousProgress;
	InOutResult.RequestedProgress = RequestedProgress;
	InOutResult.AppliedProgress = AppliedProgress;
	InOutResult.RemainingProgress = RemainingProgress;
	InOutResult.bDepletedAfterHit = bWillDeplete;
	InOutResult.bSuccess = AppliedProgress > 0.0f;

	if (!InOutResult.bSuccess)
	{
		return false;
	}

	return true;
}

void UAOHarvestableComponent::ResetHarvestNodeState()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	const bool bWasUnavailable = RuntimeState.bDepleted || RuntimeState.bRespawnPending;

	ClearRespawnTimer();
	RefreshRuntimeStateFromDefinition();

	if (bWasUnavailable && CanAcceptHarvestRequest())
	{
		BroadcastNodeRespawned();
	}
}

void UAOHarvestableComponent::MulticastPlayHarvestCue_Implementation(FVector_NetQuantize CueLocation, FVector_NetQuantizeNormal CueNormal, bool bDepletedAfterHit)
{
	if (HarvestableDefinition == nullptr)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FGameplayCueParameters CueParameters;
	CueParameters.Location = CueLocation;
	CueParameters.Normal = FVector(CueNormal).GetSafeNormal();
	CueParameters.SourceObject = HarvestableDefinition;
	UAOHarvestGameplayCueNotify_Burst::PlayDefinitionVisuals(World, HarvestableDefinition, CueParameters, bDepletedAfterHit);
}

void UAOHarvestableComponent::OnRep_RuntimeState(const FAOHarvestNodeRuntimeState& PreviousState)
{
	if ((PreviousState.bDepleted || PreviousState.bRespawnPending) && CanAcceptHarvestRequest())
	{
		BroadcastNodeRespawned();
	}
}

void UAOHarvestableComponent::OnRep_LastDepletedEvent()
{
	if (LastDepletedEvent.Sequence <= 0)
	{
		return;
	}

	BroadcastNodeDepleted(BuildLifecycleContextFromReplicatedDepletedEvent());
}

void UAOHarvestableComponent::BroadcastNodeDepleted(const FAOHarvestLifecycleContext& LifecycleContext)
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr || !OwnerActor->GetClass()->ImplementsInterface(UAOHarvestableTarget::StaticClass()))
	{
		return;
	}

	IAOHarvestableTarget::Execute_HandleHarvestNodeDepleted(OwnerActor, LifecycleContext);
}

void UAOHarvestableComponent::BroadcastNodeRespawned()
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr || !OwnerActor->GetClass()->ImplementsInterface(UAOHarvestableTarget::StaticClass()))
	{
		return;
	}

	IAOHarvestableTarget::Execute_HandleHarvestNodeRespawned(OwnerActor);
}

void UAOHarvestableComponent::RefreshRuntimeStateFromDefinition()
{
	RuntimeState.CurrentProgress = GetTotalHarvestProgress();
	RuntimeState.bDepleted = RuntimeState.CurrentProgress <= 0.0f;
	RuntimeState.bRespawnPending = false;
}

void UAOHarvestableComponent::UpdateReplicatedDepletedEvent(const FAOHarvestLifecycleContext& LifecycleContext)
{
	++LastDepletedEvent.Sequence;
	LastDepletedEvent.PreviousProgress = LifecycleContext.HarvestResult.PreviousProgress;
	LastDepletedEvent.RemainingProgress = LifecycleContext.HarvestResult.RemainingProgress;
	LastDepletedEvent.AppliedProgress = LifecycleContext.HarvestResult.AppliedProgress;
	LastDepletedEvent.bSuccess = LifecycleContext.HarvestResult.bSuccess;
	LastDepletedEvent.bDepletedAfterHit = LifecycleContext.HarvestResult.bDepletedAfterHit;
	LastDepletedEvent.HarvesterActor = LifecycleContext.HarvesterActor;
	LastDepletedEvent.HarvesterForward = LifecycleContext.HarvesterForward;
	LastDepletedEvent.bHasHarvesterForward = LifecycleContext.bHasHarvesterForward;
	LastDepletedEvent.HitLocation = LifecycleContext.HitLocation;
	LastDepletedEvent.HitNormal = LifecycleContext.HitNormal;
	LastDepletedEvent.HitDirection = LifecycleContext.HitDirection;
	LastDepletedEvent.bHasHitData = LifecycleContext.bHasHitData;
}

FAOHarvestLifecycleContext UAOHarvestableComponent::BuildLifecycleContextFromReplicatedDepletedEvent() const
{
	FAOHarvestLifecycleContext LifecycleContext;
	LifecycleContext.HarvestResult.bSuccess = LastDepletedEvent.bSuccess;
	LifecycleContext.HarvestResult.PreviousProgress = LastDepletedEvent.PreviousProgress;
	LifecycleContext.HarvestResult.RemainingProgress = LastDepletedEvent.RemainingProgress;
	LifecycleContext.HarvestResult.AppliedProgress = LastDepletedEvent.AppliedProgress;
	LifecycleContext.HarvestResult.bDepletedAfterHit = LastDepletedEvent.bDepletedAfterHit;
	LifecycleContext.HarvesterActor = LastDepletedEvent.HarvesterActor;
	LifecycleContext.HarvesterForward = LastDepletedEvent.HarvesterForward;
	LifecycleContext.bHasHarvesterForward = LastDepletedEvent.bHasHarvesterForward;
	LifecycleContext.HitLocation = LastDepletedEvent.HitLocation;
	LifecycleContext.HitNormal = LastDepletedEvent.HitNormal;
	LifecycleContext.HitDirection = LastDepletedEvent.HitDirection;
	LifecycleContext.bHasHitData = LastDepletedEvent.bHasHitData;
	return LifecycleContext;
}

void UAOHarvestableComponent::StartRespawnTimerIfNeeded()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || HarvestableDefinition == nullptr)
	{
		return;
	}

	const FAOHarvestRespawnConfig& RespawnConfig = HarvestableDefinition->RespawnConfig;
	if (!RespawnConfig.bCanRespawn)
	{
		RuntimeState.bRespawnPending = false;
		return;
	}

	RuntimeState.bRespawnPending = true;

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(RespawnTimerHandle);
	World->GetTimerManager().SetTimer(RespawnTimerHandle, this, &ThisClass::HandleRespawnTimerFinished, FMath::Max(0.0f, RespawnConfig.RespawnInterval), false);
}

void UAOHarvestableComponent::ClearRespawnTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RespawnTimerHandle);
	}
}

void UAOHarvestableComponent::HandleRespawnTimerFinished()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	ResetHarvestNodeState();
}
