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
	bContinuous = false;
}

UAT_WaitRotateToDirection* UAT_WaitRotateToDirection::WaitRotateToDirection(UGameplayAbility* OwningAbility, FRotator InTargetRotation, float InInterpSpeed, bool bInContinuous)
{
	UAT_WaitRotateToDirection* MyObj = NewAbilityTask<UAT_WaitRotateToDirection>(OwningAbility);
	MyObj->TargetRotation = InTargetRotation;
	MyObj->InterpSpeed = InInterpSpeed;
	MyObj->bContinuous = bInContinuous;
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

		APawn* Pawn = Cast<APawn>(OwnerActor);
		if (!Pawn)
		{
			EndTask();
			return;
		}

		APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
		if (!PC)
		{
			EndTask();
			return;
		}

		if (bContinuous)
		{
			FRotator ControlRotation = PC->GetControlRotation();
			TargetRotation = FRotator(0.0f, ControlRotation.Yaw, 0.0f);
		}

		FRotator CurrentRotation = OwnerActor->GetActorRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InterpSpeed);
		
		OwnerActor->SetActorRotation(NewRotation);

		float CurrentYaw = FRotator::NormalizeAxis(CurrentRotation.Yaw);
		float TargetYaw = FRotator::NormalizeAxis(TargetRotation.Yaw);
		float YawDifference = FMath::Abs(CurrentYaw - TargetYaw);
		
		UE_LOG(LogAegisOdysseyAbilitySystem, VeryVerbose, TEXT("UAT_WaitRotateToDirection::TickTask: Current Yaw=%.2f, Target Yaw=%.2f, Diff=%.2f, InterpSpeed=%.2f, Continuous=%d"), 
			CurrentYaw, TargetYaw, YawDifference, InterpSpeed, bContinuous);
		
		if (!bContinuous && YawDifference < 1.0f)
		{
			OwnerActor->SetActorRotation(TargetRotation);
			EndTask();
		}
	}
}
