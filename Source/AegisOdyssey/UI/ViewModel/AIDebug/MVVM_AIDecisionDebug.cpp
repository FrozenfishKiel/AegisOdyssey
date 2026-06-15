// Fill out your copyright notice in the Description page of Project Settings.

#include "MVVM_AIDecisionDebug.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_AIDecisionDebug)

void UMVVM_AIDecisionDebug::ApplyDebugSnapshot(const FAOAIDecisionDebugSnapshot& InDebugSnapshot)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bTrackingAI, InDebugSnapshot.bIsTrackingAI))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsTrackingAI);
	}

	if (UE_MVVM_SET_PROPERTY_VALUE(TrackedActorName, InDebugSnapshot.TrackedActorName))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetTrackedActorName);
	}

	if (UE_MVVM_SET_PROPERTY_VALUE(DecisionQueueCount, InDebugSnapshot.DecisionQueueCount))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDecisionQueueCount);
	}
	if (UE_MVVM_SET_PROPERTY_VALUE(SelectedIntentTag, InDebugSnapshot.SelectedIntentTag))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSelectedIntentTag);
	}
	if (UE_MVVM_SET_PROPERTY_VALUE(bHasCurrentEvaluationInventoryDecision, InDebugSnapshot.bHasCurrentEvaluationInventoryDecision))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasCurrentEvaluationInventoryDecision);
	}
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentEvaluationInventoryActionTag, InDebugSnapshot.CurrentEvaluationInventoryActionTag))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentEvaluationInventoryActionTag);
	}

	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentQueuedDecisionTag, InDebugSnapshot.CurrentQueuedDecisionTag))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentQueuedDecisionTag);
	}
	if (UE_MVVM_SET_PROPERTY_VALUE(CurrentSubmittedDecisionTag, InDebugSnapshot.CurrentSubmittedDecisionTag))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentSubmittedDecisionTag);
	}
	if (UE_MVVM_SET_PROPERTY_VALUE(LastSubmittedDecisionTag, InDebugSnapshot.LastSubmittedDecisionTag))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetLastSubmittedDecisionTag);
	}

	if (UE_MVVM_SET_PROPERTY_VALUE(
		bHasCurrentSubmittedInventoryDecision,
		InDebugSnapshot.bHasCurrentSubmittedInventoryDecision))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasCurrentSubmittedInventoryDecision);
	}

	if (UE_MVVM_SET_PROPERTY_VALUE(
		CurrentSubmittedInventoryActionTag,
		InDebugSnapshot.CurrentSubmittedInventoryDecision.ActionTag))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCurrentSubmittedInventoryActionTag);
	}
	if (UE_MVVM_SET_PROPERTY_VALUE(PendingSubmitDelaySeconds, InDebugSnapshot.PendingSubmitDelaySeconds))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetPendingSubmitDelaySeconds);
	}
}
