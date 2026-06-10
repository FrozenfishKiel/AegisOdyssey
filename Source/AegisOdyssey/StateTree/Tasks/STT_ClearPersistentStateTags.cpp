// Fill out your copyright notice in the Description page of Project Settings.

#include "STT_ClearPersistentStateTags.h"

#include "AegisOdyssey/Character/AOPersistentStateTagComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_ClearPersistentStateTags)

namespace
{
AActor* NormalizePersistentStateTagClearTarget(AActor* TargetActor)
{
	if (AController* Controller = Cast<AController>(TargetActor))
	{
		if (APawn* Pawn = Controller->GetPawn())
		{
			return Pawn;
		}
	}

	if (APawn* Pawn = Cast<APawn>(TargetActor))
	{
		return Pawn;
	}

	return TargetActor;
}
}

EStateTreeRunStatus FSTT_ClearPersistentStateTags::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bCleared = false;

	if (InstanceData.SourceId.IsNone())
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_ClearPersistentStateTags::EnterState: SourceId is invalid."));
		return EStateTreeRunStatus::Failed;
	}

	AActor* TargetActor = ResolveTargetActor(Context, InstanceData);
	if (TargetActor == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_ClearPersistentStateTags::EnterState: Failed to resolve target actor."));
		return EStateTreeRunStatus::Failed;
	}

	UAOPersistentStateTagComponent* PersistentStateTagComponent = UAOPersistentStateTagComponent::FindPersistentStateTagComponent(TargetActor);
	if (PersistentStateTagComponent == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_ClearPersistentStateTags::EnterState: %s has no PersistentStateTagComponent."),
			*GetNameSafe(TargetActor));
		return EStateTreeRunStatus::Failed;
	}

	if (!PersistentStateTagComponent->HasSource(InstanceData.SourceId))
	{
		InstanceData.bCleared = true;
		return EStateTreeRunStatus::Succeeded;
	}

	InstanceData.bCleared = PersistentStateTagComponent->ClearTagsBySource(InstanceData.SourceId);
	return InstanceData.bCleared ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

AActor* FSTT_ClearPersistentStateTags::ResolveTargetActor(const FStateTreeExecutionContext& Context, const FInstanceDataType& InstanceData) const
{
	if (InstanceData.TargetActor != nullptr)
	{
		return NormalizePersistentStateTagClearTarget(InstanceData.TargetActor);
	}

	return NormalizePersistentStateTagClearTarget(Cast<AActor>(Context.GetOwner()));
}
