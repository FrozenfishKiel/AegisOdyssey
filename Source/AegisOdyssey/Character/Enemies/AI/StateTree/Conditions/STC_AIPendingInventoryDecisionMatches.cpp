// Fill out your copyright notice in the Description page of Project Settings.

#include "STC_AIPendingInventoryDecisionMatches.h"

#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STC_AIPendingInventoryDecisionMatches)

namespace
{
bool DoesCoordinationModeMatch(
	const EAOAIInventoryDecisionCoordinationFilter CoordinationFilter,
	const EAOAIInventoryActionCoordinationMode CoordinationMode)
{
	switch (CoordinationFilter)
	{
	case EAOAIInventoryDecisionCoordinationFilter::Any:
		return true;
	case EAOAIInventoryDecisionCoordinationFilter::Exclusive:
		return CoordinationMode == EAOAIInventoryActionCoordinationMode::Exclusive;
	case EAOAIInventoryDecisionCoordinationFilter::Additive:
		return CoordinationMode == EAOAIInventoryActionCoordinationMode::Additive;
	default:
		return false;
	}
}
}

bool FSTC_AIPendingInventoryDecisionMatches::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.bRequirePendingInventoryDecision && !InstanceData.bHasCurrentSubmittedInventoryDecision)
	{
		return false ^ bInvert;
	}

	if (InstanceData.bRequireAdditiveInventoryWindow && !InstanceData.bHasAdditiveInventoryWindow)
	{
		return false ^ bInvert;
	}

	if (!InstanceData.bHasCurrentSubmittedInventoryDecision)
	{
		return true ^ bInvert;
	}

	const FAOAIInventoryDecisionResult& CurrentDecision = InstanceData.CurrentSubmittedInventoryDecision;
	if (!DoesCoordinationModeMatch(InstanceData.CoordinationFilter, CurrentDecision.CoordinationMode))
	{
		return false ^ bInvert;
	}

	if (InstanceData.ExpectedActionTag.IsValid()
		&& !CurrentDecision.ActionTag.MatchesTagExact(InstanceData.ExpectedActionTag))
	{
		return false ^ bInvert;
	}

	if (InstanceData.ExpectedCandidateTag.IsValid()
		&& !CurrentDecision.CandidateTag.MatchesTagExact(InstanceData.ExpectedCandidateTag))
	{
		return false ^ bInvert;
	}

	if (InstanceData.bRequireResolvedTarget && !CurrentDecision.bHasResolvedTarget)
	{
		return false ^ bInvert;
	}

	return true ^ bInvert;
}
