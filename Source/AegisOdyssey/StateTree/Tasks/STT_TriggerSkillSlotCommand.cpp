#include "STT_TriggerSkillSlotCommand.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_TriggerSkillSlotCommand)

EStateTreeRunStatus FSTT_TriggerSkillSlotCommand::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bCommandAccepted = false;
	InstanceData.ElapsedWaitTime = 0.0f;
	InstanceData.SkillComponent = nullptr;
	InstanceData.EnterWorldTimeSeconds = GetCurrentWorldTimeSeconds(Context);

	AActor* CommandTarget = ResolveCommandTarget(Context);
	if (CommandTarget == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.SkillComponent = ResolveSkillComponent(CommandTarget);
	if (InstanceData.SkillComponent == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	const int32 ResolvedSlotIndex = ResolveSlotIndex(*InstanceData.SkillComponent, InstanceData);
	if (ResolvedSlotIndex == INDEX_NONE)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bCommandAccepted = InstanceData.SkillComponent->ExecuteSkillSlotCommandByIndex(
		ResolvedSlotIndex,
		InstanceData.InputType);
	if (!InstanceData.bCommandAccepted)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.WaitMode == ESTTCommandWaitMode::WaitFixedDuration)
	{
		return InstanceData.FixedWaitSeconds <= 0.0f ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FSTT_TriggerSkillSlotCommand::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.WaitMode != ESTTCommandWaitMode::WaitFixedDuration)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	const float CurrentWorldTimeSeconds = GetCurrentWorldTimeSeconds(Context);
	if (CurrentWorldTimeSeconds >= 0.0f && InstanceData.EnterWorldTimeSeconds >= 0.0f)
	{
		InstanceData.ElapsedWaitTime = CurrentWorldTimeSeconds - InstanceData.EnterWorldTimeSeconds;
	}
	else
	{
		InstanceData.ElapsedWaitTime += DeltaTime;
	}

	return InstanceData.ElapsedWaitTime >= InstanceData.FixedWaitSeconds
		? EStateTreeRunStatus::Succeeded
		: EStateTreeRunStatus::Running;
}

AActor* FSTT_TriggerSkillSlotCommand::ResolveCommandTarget(const FStateTreeExecutionContext& Context) const
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

	if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
	{
		return OwnerPawn;
	}

	return OwnerActor;
}

UAOSkillComponent* FSTT_TriggerSkillSlotCommand::ResolveSkillComponent(AActor* CommandTarget) const
{
	return UAOSkillComponent::FindSkillComponent(CommandTarget);
}

int32 FSTT_TriggerSkillSlotCommand::ResolveSlotIndex(const UAOSkillComponent& SkillComponent, const FInstanceDataType& InstanceData) const
{
	if (InstanceData.SlotIndex != INDEX_NONE)
	{
		return SkillComponent.IsValidSkillSlotIndex(InstanceData.SlotIndex) ? InstanceData.SlotIndex : INDEX_NONE;
	}

	if (InstanceData.SkillSlotInputTag.IsValid())
	{
		return SkillComponent.FindSkillSlotIndexByInputTag(InstanceData.SkillSlotInputTag);
	}

	return INDEX_NONE;
}

float FSTT_TriggerSkillSlotCommand::GetCurrentWorldTimeSeconds(const FStateTreeExecutionContext& Context) const
{
	if (AActor* OwnerActor = Cast<AActor>(Context.GetOwner()))
	{
		if (const UWorld* World = OwnerActor->GetWorld())
		{
			return World->GetTimeSeconds();
		}
	}

	return -1.0f;
}
