// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/Character/Enemies/AI/StateTree/Tasks/STT_UseResolvedInventoryItem.h"

#include "AIController.h"
#include "AegisOdyssey/Character/Enemies/AI/AOAIInventoryRuntimeUseLibrary.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_UseResolvedInventoryItem)

EStateTreeRunStatus FSTT_UseResolvedInventoryItem::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bUseSucceeded = false;
	InstanceData.ExecutionResult = FAOAIInventoryUseExecutionResult();

	APawn* UserPawn = ResolveUserPawn(Context);
	if (UserPawn == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_UseResolvedInventoryItem::EnterState: Failed to resolve user pawn."));
		return EStateTreeRunStatus::Failed;
	}

	FAOAIInventoryDecisionResult ResolvedDecisionResult;
	FAOAIInventoryUseCommand ResolvedUseCommand;
	if (!ResolveUseCommand(Context, UserPawn, InstanceData, ResolvedDecisionResult, ResolvedUseCommand))
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_UseResolvedInventoryItem::EnterState: Failed to resolve use command."));
		return EStateTreeRunStatus::Failed;
	}

	if (ResolvedDecisionResult.bHasResolvedTarget)
	{
		InstanceData.bUseSucceeded = UAOAIInventoryRuntimeUseLibrary::TryExecuteResolvedTarget(
			UserPawn,
			ResolvedDecisionResult.ResolvedTarget,
			InstanceData.ExecutionResult);
	}
	else
	{
		InstanceData.bUseSucceeded =
			UAOAIInventoryRuntimeUseLibrary::TryExecuteUseCommand(UserPawn, ResolvedUseCommand, InstanceData.ExecutionResult);
	}

	if (!InstanceData.bUseSucceeded)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (UAOAIDecisionComponent* DecisionComponent = UAOAIDecisionComponent::FindAIDecisionComponent(UserPawn))
	{
		const UWorld* World = UserPawn->GetWorld();
		DecisionComponent->CommitExecutedInventoryAction(ResolvedDecisionResult, World ? World->GetTimeSeconds() : -1.0f);
	}

	return EStateTreeRunStatus::Succeeded;
}

APawn* FSTT_UseResolvedInventoryItem::ResolveUserPawn(const FStateTreeExecutionContext& Context) const
{
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (OwnerActor == nullptr)
	{
		return nullptr;
	}

	if (AAIController* AIController = Cast<AAIController>(OwnerActor))
	{
		return AIController->GetPawn();
	}

	return Cast<APawn>(OwnerActor);
}

bool FSTT_UseResolvedInventoryItem::ResolveUseCommand(
	const FStateTreeExecutionContext& Context,
	APawn* UserPawn,
	const FInstanceDataType& InstanceData,
	FAOAIInventoryDecisionResult& OutResolvedDecisionResult,
	FAOAIInventoryUseCommand& OutUseCommand) const
{
	OutResolvedDecisionResult = FAOAIInventoryDecisionResult();
	OutUseCommand = InstanceData.UseCommand;

	if (!InstanceData.bPreferDecisionPendingCommand)
	{
		OutResolvedDecisionResult.bHasAction = true;
		OutResolvedDecisionResult.UseCommand = OutUseCommand;
		return true;
	}

	if (InstanceData.CurrentSubmittedInventoryDecision.bHasAction)
	{
		OutResolvedDecisionResult = InstanceData.CurrentSubmittedInventoryDecision;
		OutUseCommand = OutResolvedDecisionResult.UseCommand;
		return true;
	}

	if (UAOAIDecisionComponent* DecisionComponent = UAOAIDecisionComponent::FindAIDecisionComponent(UserPawn))
	{
		if (DecisionComponent->GetCurrentSubmittedInventoryDecisionResult(OutResolvedDecisionResult))
		{
			OutUseCommand = OutResolvedDecisionResult.UseCommand;
			return true;
		}
	}

	return false;
}
