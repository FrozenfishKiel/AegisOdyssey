// Fill out your copyright notice in the Description page of Project Settings.

#include "STC_AIDecisionIntentMatches.h"

#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STC_AIDecisionIntentMatches)

bool FSTC_AIDecisionIntentMatches::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	const UAOAIDecisionComponent* DecisionComponent = UAOAIDecisionComponent::FindAIDecisionComponent(OwnerActor);

	// 第四阶段开始，这个 Condition 优先消费统一提交主链里的正式结果。
	// 如果当前资源还没完全切完，再兼容回退到旧的 SelectedIntentTag。
	const bool bMatches = InstanceData.ExpectedIntentTag.IsValid()
		&& DecisionComponent != nullptr
		&& DecisionComponent->MatchesCurrentDecisionTag(InstanceData.ExpectedIntentTag);

	return bMatches ^ bInvert;
}
