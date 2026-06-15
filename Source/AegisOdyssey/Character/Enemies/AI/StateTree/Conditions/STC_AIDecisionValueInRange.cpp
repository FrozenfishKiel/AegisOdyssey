// Fill out your copyright notice in the Description page of Project Settings.

#include "STC_AIDecisionValueInRange.h"

#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STC_AIDecisionValueInRange)

bool FSTC_AIDecisionValueInRange::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	const UAOAIDecisionComponent* DecisionComponent = UAOAIDecisionComponent::FindAIDecisionComponent(OwnerActor);
	if (DecisionComponent == nullptr)
	{
		return false ^ bInvert;
	}

	float Value = 0.0f;
	switch (InstanceData.ValueType)
	{
	case EAOAIDecisionValueType::IntentDesire:
	case EAOAIDecisionValueType::IntentScore:
	{
		float Desire = 0.0f;
		float Score = 0.0f;
		const bool bHasMetrics = DecisionComponent->GetIntentRuntimeMetrics(InstanceData.IntentTag, Desire, Score);
		if (!bHasMetrics)
		{
			return false ^ bInvert;
		}

		Value = InstanceData.ValueType == EAOAIDecisionValueType::IntentDesire ? Desire : Score;
		break;
	}
	case EAOAIDecisionValueType::RepeatedIntentCount:
		Value = static_cast<float>(DecisionComponent->GetRepeatedIntentCount());
		break;
	default:
		return false ^ bInvert;
	}

	// 这个 Condition 现在不再写死 Attack / Strafe 这类分支，
	// 而是统一按 Tag 去查询任意意图的运行时数据。
	const float MinValue = FMath::Min(InstanceData.MinValue, InstanceData.MaxValue);
	const float MaxValue = FMath::Max(InstanceData.MinValue, InstanceData.MaxValue);
	const bool bInRange = Value >= MinValue && Value <= MaxValue;
	return bInRange ^ bInvert;
}
