// Fill out your copyright notice in the Description page of Project Settings.

#include "STT_ResetAIDecisionState.h"

#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_ResetAIDecisionState)

EStateTreeRunStatus FSTT_ResetAIDecisionState::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bResetSucceeded = false;

	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	UAOAIDecisionComponent* DecisionComponent = UAOAIDecisionComponent::FindAIDecisionComponent(OwnerActor);
	if (DecisionComponent == nullptr)
	{
		return bSucceedIfComponentMissing ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
	}

	// 这个 Task 适合放在脱战、回归巡逻，或者你想显式切断上一轮战斗记忆的状态里。
	DecisionComponent->ResetDecisionState();
	InstanceData.bResetSucceeded = true;
	return EStateTreeRunStatus::Succeeded;
}
