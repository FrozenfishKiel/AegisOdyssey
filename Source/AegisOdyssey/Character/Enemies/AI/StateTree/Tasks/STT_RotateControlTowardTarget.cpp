#include "STT_RotateControlTowardTarget.h"

#include "AegisOdyssey/Player/AAOAIPlayerBotController.h"
#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STT_RotateControlTowardTarget)

FSTT_RotateControlTowardTarget::FSTT_RotateControlTowardTarget()
{
	bShouldCallTick = true;
	bShouldCopyBoundPropertiesOnTick = true;
	bShouldCopyBoundPropertiesOnExitState = false;
}

EStateTreeRunStatus FSTT_RotateControlTowardTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.ElapsedTime = 0.0f;
	InstanceData.CurrentYawDifference = 0.0f;
	InstanceData.bReachedTargetYaw = false;
	InstanceData.StartTimeSeconds = GetCurrentWorldTimeSeconds(Context);
	InstanceData.AIController = ResolveAIController(Context);

	if (InstanceData.AIController == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_RotateControlTowardTarget::EnterState: AIController is missing."));
		return EStateTreeRunStatus::Failed;
	}

	if (ResolveTargetActor(Context) == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("FSTT_RotateControlTowardTarget::EnterState: TargetActor is missing."));
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_RotateControlTowardTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const float CurrentWorldTimeSeconds = GetCurrentWorldTimeSeconds(Context);
	if (CurrentWorldTimeSeconds >= 0.0f && InstanceData.StartTimeSeconds >= 0.0f)
	{
		InstanceData.ElapsedTime = CurrentWorldTimeSeconds - InstanceData.StartTimeSeconds;
	}
	else
	{
		InstanceData.ElapsedTime += DeltaTime;
	}

	if (InstanceData.MaxDuration > 0.0f && InstanceData.ElapsedTime >= InstanceData.MaxDuration)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (InstanceData.AIController == nullptr)
	{
		InstanceData.AIController = ResolveAIController(Context);
	}

	if (InstanceData.AIController == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* ControlledPawn = InstanceData.AIController->GetPawn();
	AActor* TargetActor = ResolveTargetActor(Context);
	if (ControlledPawn == nullptr || TargetActor == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	FVector TargetDirection = TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation();
	if (!InstanceData.bUsePitch)
	{
		TargetDirection.Z = 0.0f;
	}

	if (TargetDirection.IsNearlyZero())
	{
		InstanceData.CurrentYawDifference = 0.0f;
		InstanceData.bReachedTargetYaw = true;
		return InstanceData.bContinuous ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
	}

	FRotator DesiredRotation = TargetDirection.Rotation();
	DesiredRotation.Yaw = FRotator::NormalizeAxis(DesiredRotation.Yaw + InstanceData.AimOffsetYaw);

	const FRotator CurrentControlRotation = InstanceData.AIController->GetControlRotation();
	if (!InstanceData.bUsePitch)
	{
		DesiredRotation.Pitch = CurrentControlRotation.Pitch;
	}
	DesiredRotation.Roll = 0.0f;

	const float EffectiveRotationSpeed = FMath::Max(0.0f, InstanceData.RotationSpeed * InstanceData.RotationSpeedMultiplier);
	const FRotator NewControlRotation = FMath::RInterpConstantTo(CurrentControlRotation, DesiredRotation, DeltaTime, EffectiveRotationSpeed);
	InstanceData.AIController->SetControlRotation(NewControlRotation);

	InstanceData.CurrentYawDifference = FMath::Abs(FRotator::NormalizeAxis(DesiredRotation.Yaw - NewControlRotation.Yaw));
	InstanceData.bReachedTargetYaw = (InstanceData.CurrentYawDifference <= InstanceData.YawTolerance);

	if (!InstanceData.bContinuous && InstanceData.bReachedTargetYaw)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

AAIController* FSTT_RotateControlTowardTarget::ResolveAIController(const FStateTreeExecutionContext& Context) const
{
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (OwnerActor == nullptr)
	{
		return nullptr;
	}

	if (AAIController* DirectController = Cast<AAIController>(OwnerActor))
	{
		return DirectController;
	}

	if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
	{
		return Cast<AAIController>(OwnerPawn->GetController());
	}

	return nullptr;
}

AActor* FSTT_RotateControlTowardTarget::ResolveTargetActor(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.TargetActor != nullptr)
	{
		return InstanceData.TargetActor;
	}

	if (AAOAIPlayerBotController* AOAIController = Cast<AAOAIPlayerBotController>(InstanceData.AIController))
	{
		return AOAIController->GetCurrentTarget();
	}

	return nullptr;
}

float FSTT_RotateControlTowardTarget::GetCurrentWorldTimeSeconds(const FStateTreeExecutionContext& Context) const
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
