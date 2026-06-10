// Fill out your copyright notice in the Description page of Project Settings.

#include "STT_GrantPersistentStateTags.h"

#include "AegisOdyssey/Character/AOPersistentStateTagComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_GrantPersistentStateTags)

namespace
{
AActor* NormalizePersistentStateTagTarget(AActor* TargetActor)
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

EStateTreeRunStatus FSTT_GrantPersistentStateTags::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bGranted = false;

	AActor* TargetActor = ResolveTargetActor(Context, InstanceData);
	if (TargetActor == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_GrantPersistentStateTags::EnterState: Failed to resolve target actor."));
		return EStateTreeRunStatus::Failed;
	}

	UAOPersistentStateTagComponent* PersistentStateTagComponent = UAOPersistentStateTagComponent::FindPersistentStateTagComponent(TargetActor);
	if (PersistentStateTagComponent == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_GrantPersistentStateTags::EnterState: %s has no PersistentStateTagComponent."),
			*GetNameSafe(TargetActor));
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bGranted = PersistentStateTagComponent->EnsureTagsGranted(InstanceData.SourceId, InstanceData.GrantedTags);
	return InstanceData.bGranted ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

AActor* FSTT_GrantPersistentStateTags::ResolveTargetActor(const FStateTreeExecutionContext& Context, const FInstanceDataType& InstanceData) const
{
	if (InstanceData.TargetActor != nullptr)
	{
		return NormalizePersistentStateTagTarget(InstanceData.TargetActor);
	}

	return NormalizePersistentStateTagTarget(Cast<AActor>(Context.GetOwner()));
}
