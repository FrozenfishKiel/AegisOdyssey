// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Roll.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/AbilitySystem/Tasks/AT_WaitRotateToDirection.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_Roll)

void UGA_Roll::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

bool UGA_Roll::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return false;
	}

	return true;
}

void UGA_Roll::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData)
	{
		for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TriggerEventData->TargetData.Data)
		{
			if (Data.IsValid() && Data->GetScriptStruct() == FRollTargetData::StaticStruct())
			{
				FRollTargetData* RollTargetData = static_cast<FRollTargetData*>(Data.Get());
				
				/**
				 * 创建参数对象并填充数据
				 * 客户端和服务器都会执行这段代码，获取到相同的参数
				 */
				InputTag = RollTargetData->InputTag;
				InputType = RollTargetData->InputType;
				Montage = RollTargetData->Montage.Get();
				PlayRate = RollTargetData->PlayRate;
				StartSection = RollTargetData->StartSection;
				StartTime = RollTargetData->StartTime;
				
				break;
			}
		}
	}

	SetCharacterRotationToAttackDirection();

	PlayMontageAnimation();
}

void UGA_Roll::PlayMontageAnimation()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::PlayMontageAnimation: AnimInstance: %s"), 
		*GetNameSafe(AnimInstance));
    
	if (!AnimInstance)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_LightAttack::PlayMontageAnimation: AnimInstance is null!"));
		return;
	}
	

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("PlayMontageAndWait"),
		Montage,
		PlayRate,
		StartSection,
		true,
		1.0f,
		StartTime,
		false
	);

	if (MontageTask)
	{
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_Roll::OnMontageBlendedOut);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_Roll::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Roll::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_Roll::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
}

void UGA_Roll::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}

void UGA_Roll::OnMontageBlendedOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}

void UGA_Roll::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}

void UGA_Roll::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}

/*
* 假设摄像机朝向北方（Y轴正方向），角色朝向南方（Y轴负方向）：

情况1：按 W 键（朝摄像机前方）

- MoveInputVector = (0, 1, 0)（世界坐标系，朝北）
- ForwardDirection = (0, 1, 0)（摄像机前向，朝北）
- RightDirection = (1, 0, 0)（摄像机右向，朝东）
- ForwardDot = 0×0 + 1×1 + 0×0 = 1
- RightDot = 0×1 + 1×0 + 0×0 = 0
- CameraRelativeDirection = (0, 1, 0)×1 + (1, 0, 0)×0 = (0, 1, 0)
- 结果：朝摄像机前方翻滚 ✓
情况2：按 S 键（朝摄像机后方）

- MoveInputVector = (0, -1, 0)（世界坐标系，朝南）
- ForwardDirection = (0, 1, 0)（摄像机前向，朝北）
- RightDirection = (1, 0, 0)（摄像机右向，朝东）
- ForwardDot = 0×0 + (-1)×1 + 0×0 = -1
- RightDot = 0×1 + (-1)×0 + 0×0 = 0
- CameraRelativeDirection = (0, 1, 0)×(-1) + (1, 0, 0)×0 = (0, -1, 0)
- 结果：朝摄像机后方翻滚 ✓
情况3：按 W+A 键（朝摄像机左前方）

- MoveInputVector = (-0.707, 0.707, 0)（世界坐标系，朝西北）
- ForwardDirection = (0, 1, 0)（摄像机前向，朝北）
- RightDirection = (1, 0, 0)（摄像机右向，朝东）
- ForwardDot = (-0.707)×0 + 0.707×1 + 0×0 = 0.707
- RightDot = (-0.707)×1 + 0.707×0 + 0×0 = -0.707
- CameraRelativeDirection = (0, 1, 0)×0.707 + (1, 0, 0)×(-0.707) = (-0.707, 0.707, 0)
- 结果：朝摄像机左前方翻滚 ✓
 */
void UGA_Roll::SetCharacterRotationToAttackDirection()
{
	if (!CurrentActorInfo)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(CurrentActorInfo->AvatarActor);
	if (!Pawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC)
	{
		return;
	}

	FVector MoveInputVector = FVector::ZeroVector;
	if (ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		MoveInputVector = Character->GetLastMovementInputVector();
	}

	FRotator TargetRotation;
	constexpr float InputDeadZone = 0.1f;

	if (MoveInputVector.Size() > InputDeadZone)
	{
		FRotator ControlRotation = PC->GetControlRotation();
		FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
		
		FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		float ForwardDot = FVector::DotProduct(MoveInputVector, ForwardDirection);
		float RightDot = FVector::DotProduct(MoveInputVector, RightDirection);
		
		FVector CameraRelativeDirection = ForwardDirection * ForwardDot + RightDirection * RightDot;
		CameraRelativeDirection = CameraRelativeDirection.GetSafeNormal();
		
		TargetRotation = CameraRelativeDirection.Rotation();
		TargetRotation.Pitch = 0.0f;
		TargetRotation.Roll = 0.0f;
		
		UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SetCharacterRotationToAttackDirection: Rolling in camera-relative direction: Forward=%.2f, Right=%.2f, Yaw=%.2f"), 
			ForwardDot, RightDot, TargetRotation.Yaw);
	}
	else
	{
		TargetRotation = Pawn->GetActorRotation();
		TargetRotation.Pitch = 0.0f;
		TargetRotation.Roll = 0.0f;
		
		UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SetCharacterRotationToAttackDirection: No input, rolling in character facing direction: Yaw=%.2f"), TargetRotation.Yaw);
	}

	if (!RotationTask)
	{
		RotationTask = UAT_WaitRotateToDirection::WaitRotateToDirection(this, TargetRotation, RotationInterpSpeed);
		if (RotationTask)
		{
			RotationTask->ReadyForActivation();
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SetCharacterRotationToAttackDirection: Started rotation to Yaw: %.2f with speed: %.2f"), 
				TargetRotation.Yaw, RotationInterpSpeed);
		}
	}
}

void UGA_Roll::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                          const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (MontageTask)
	{
		MontageTask->EndTask();
		Montage = nullptr;
	}
	if (RotationTask)
	{
		RotationTask->EndTask();
		RotationTask = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

