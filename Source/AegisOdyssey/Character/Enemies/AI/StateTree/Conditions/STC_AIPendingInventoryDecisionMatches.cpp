// Fill out your copyright notice in the Description page of Project Settings.

#include "STC_AIPendingInventoryDecisionMatches.h"

#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STC_AIPendingInventoryDecisionMatches)

bool FSTC_AIPendingInventoryDecisionMatches::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.bRequirePendingInventoryDecision && !InstanceData.bHasCurrentSubmittedInventoryDecision)
	{
		return false ^ bInvert;
	}

	if (!InstanceData.bHasCurrentSubmittedInventoryDecision)
	{
		return true ^ bInvert;
	}

	const FAOAIInventoryDecisionResult& CurrentDecision = InstanceData.CurrentSubmittedInventoryDecision;
	if (InstanceData.ExpectedActionTag.IsValid()
		&& !CurrentDecision.ActionTag.MatchesTagExact(InstanceData.ExpectedActionTag))
	{
		return false ^ bInvert;
	}

	return true ^ bInvert;
}
