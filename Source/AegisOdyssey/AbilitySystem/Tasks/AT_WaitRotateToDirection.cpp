#include "AT_WaitRotateToDirection.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AT_WaitRotateToDirection)

UAT_WaitRotateToDirection::UAT_WaitRotateToDirection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
	InterpSpeed = 360.0f;
	TargetRotation = FRotator::ZeroRotator;
}

UAT_WaitRotateToDirection* UAT_WaitRotateToDirection::WaitRotateToDirection(UGameplayAbility* OwningAbility, FRotator InTargetRotation, float InInterpSpeed)
{
	UAT_WaitRotateToDirection* MyObj = NewAbilityTask<UAT_WaitRotateToDirection>(OwningAbility);
	MyObj->TargetRotation = InTargetRotation;
	MyObj->InterpSpeed = InInterpSpeed;
	return MyObj;
}

void UAT_WaitRotateToDirection::Activate()
{
	SetWaitingOnAvatar();
}

void UAT_WaitRotateToDirection::TickTask(float DeltaTime)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (!Ability || !Ability->GetAvatarActorFromActorInfo())
		{
			EndTask();
			return;
		}

		AActor* OwnerActor = Ability->GetAvatarActorFromActorInfo();
		if (!OwnerActor)
		{
			EndTask();
			return;
		}

		FRotator CurrentRotation = OwnerActor->GetActorRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InterpSpeed);
		
		OwnerActor->SetActorRotation(NewRotation);

		float CurrentYaw = FRotator::NormalizeAxis(CurrentRotation.Yaw);
		float TargetYaw = FRotator::NormalizeAxis(TargetRotation.Yaw);
		float YawDifference = FMath::Abs(CurrentYaw - TargetYaw);
		
		UE_LOG(LogAegisOdysseyAbilitySystem, VeryVerbose, TEXT("UAT_WaitRotateToDirection::TickTask: Current Yaw=%.2f, Target Yaw=%.2f, Diff=%.2f, InterpSpeed=%.2f"), 
			CurrentYaw, TargetYaw, YawDifference, InterpSpeed);
		
		if (YawDifference < 1.0f)
		{
			EndTask();
		}
	}
}
