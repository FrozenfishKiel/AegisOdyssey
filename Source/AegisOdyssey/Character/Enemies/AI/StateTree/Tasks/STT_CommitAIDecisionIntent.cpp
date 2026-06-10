// Fill out your copyright notice in the Description page of Project Settings.

#include "STT_CommitAIDecisionIntent.h"

#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_CommitAIDecisionIntent)

EStateTreeRunStatus FSTT_CommitAIDecisionIntent::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bCommitSucceeded = false;

	if (!InstanceData.ExecutedIntentTag.IsValid())
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_CommitAIDecisionIntent::EnterState: ExecutedIntentTag is invalid."));
		return EStateTreeRunStatus::Failed;
	}

	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	UAOAIDecisionComponent* DecisionComponent = UAOAIDecisionComponent::FindAIDecisionComponent(OwnerActor);
	if (DecisionComponent == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_CommitAIDecisionIntent::EnterState: %s has no AIDecisionComponent."),
			*GetNameSafe(OwnerActor));
		return EStateTreeRunStatus::Failed;
	}

	const float CurrentWorldTimeSeconds = OwnerActor && OwnerActor->GetWorld() ? OwnerActor->GetWorld()->GetTimeSeconds() : -1.0f;

	// 第四阶段开始，这里只继续承担“执行记忆回写”的兼容职责，
	// 不再把它当成战斗主链正式决策入口。
	// 这样旧的重复惩罚和节奏统计还能继续工作，但主语义已经转去统一提交流。
	InstanceData.bCommitSucceeded = DecisionComponent->CommitExecutedIntent(InstanceData.ExecutedIntentTag, CurrentWorldTimeSeconds);
	return InstanceData.bCommitSucceeded ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}
