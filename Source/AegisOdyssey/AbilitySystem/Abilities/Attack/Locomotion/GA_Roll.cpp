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
				FRollTargetData* LightAttackData = static_cast<FRollTargetData*>(Data.Get());
				
				/**
				 * 创建参数对象并填充数据
				 * 客户端和服务器都会执行这段代码，获取到相同的参数
				 */
				InputTag = LightAttackData->InputTag;
				InputType = LightAttackData->InputType;
				Montage = LightAttackData->Montage.Get();
				PlayRate = LightAttackData->PlayRate;
				StartSection = LightAttackData->StartSection;
				StartTime = LightAttackData->StartTime;
				
				break;
			}
		}
	}
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

	// 获取移动输入方向
	FVector MoveInputVector = FVector::ZeroVector;
	if (ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MoveInputVector = MovementComp->GetLastInputVector();
		}
	}

	// 计算翻滚方向
	FRotator TargetRotation;
	constexpr float InputDeadZone = 0.1f; // 输入死区阈值

	if (MoveInputVector.Size() > InputDeadZone)
	{
		// 有移动输入，使用输入方向
		// 输入向量已经是世界空间的方向（考虑了摄像机旋转）
		TargetRotation = MoveInputVector.Rotation();
		TargetRotation.Pitch = 0.0f;
		TargetRotation.Roll = 0.0f;
		
		UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SetCharacterRotationToAttackDirection: Rolling in move direction: X=%.2f, Y=%.2f"), 
			MoveInputVector.X, MoveInputVector.Y);
	}
	else
	{
		//无移动输入直接正常播放动画就行（后撤）
		return;
	}

	// 使用旋转任务平滑旋转到目标方向
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
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

