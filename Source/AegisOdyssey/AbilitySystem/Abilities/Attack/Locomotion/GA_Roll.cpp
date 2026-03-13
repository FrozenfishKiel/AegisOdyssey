// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Roll.h"
#include "NativeGameplayTags.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/AbilitySystem/Tasks/AT_WaitRotateToDirection.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_Roll)
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Roll_Cooldown, "Ability.Roll.Cooldown");

static const FGameplayTagContainer BlockCooldownTags(TAG_Ability_Roll_Cooldown);

void UGA_Roll::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

const FGameplayTagContainer* UGA_Roll::GetCooldownTags() const
{
	return &BlockCooldownTags;
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
				PlayRate = RollTargetData->PlayRate;
				StartSection = RollTargetData->StartSection;
				StartTime = RollTargetData->StartTime;
				
				ForwardMontage = RollTargetData->ForwardMontage;
				RightMontage = RollTargetData->RightMontage;
				BackwardMontage = RollTargetData->BackwardMontage;
				LeftMontage = RollTargetData->LeftMontage;
				ForwardLeftMontage = RollTargetData->ForwardLeftMontage;
				ForwardRightMontage = RollTargetData->ForwardRightMontage;
				BackwardLeftMontage = RollTargetData->BackwardLeftMontage;
				BackwardRightMontage = RollTargetData->BackwardRightMontage;
				
				SavedMoveInputDirection = RollTargetData->MoveInputDirection;
				
				break;
			}
		}
	}

	CommitAbility(Handle, ActorInfo, ActivationInfo);

	SelectDirectionalMontage();
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

/**
 * 根据输入方向选择对应的翻滚动画
 * 支持8个方向的翻滚：前、后、左、右、左前、右前、左后、右后
 */
void UGA_Roll::SelectDirectionalMontage()
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

	FVector MoveInputVector = SavedMoveInputDirection;

	constexpr float InputDeadZone = 0.1f;
	constexpr float DiagonalThreshold = 0.5f;

	if (MoveInputVector.Size() > InputDeadZone)
	{
		FRotator ControlRotation = PC->GetControlRotation();
		FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
		
		FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		float ForwardDot = FVector::DotProduct(MoveInputVector, ForwardDirection);
		float RightDot = FVector::DotProduct(MoveInputVector, RightDirection);

		FVector CameraRelativeDirection = ForwardDirection * ForwardDot + RightDirection * RightDot;
		
		FRotator TargetRotation = CameraRelativeDirection.Rotation();
		TargetRotation.Pitch = 0.0f;
		TargetRotation.Roll = 0.0f;
		
		UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: ForwardDot=%.2f, RightDot=%.2f"), 
			ForwardDot, RightDot);
		
		if (ForwardDot > DiagonalThreshold && RightDot > DiagonalThreshold)
		{
			Montage = ForwardRightMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Forward-Right Roll"));
		}
		else if (ForwardDot > DiagonalThreshold && RightDot < -DiagonalThreshold)
		{
			Montage = ForwardLeftMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Forward-Left Roll"));
		}
		else if (ForwardDot < -DiagonalThreshold && RightDot > DiagonalThreshold)
		{
			Montage = BackwardRightMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Backward-Right Roll"));
		}
		else if (ForwardDot < -DiagonalThreshold && RightDot < -DiagonalThreshold)
		{
			Montage = BackwardLeftMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Backward-Left Roll"));
		}
		else if (ForwardDot > DiagonalThreshold)
		{
			Montage = ForwardMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Forward Roll"));
		}
		else if (ForwardDot < -DiagonalThreshold)
		{
			Montage = BackwardMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Backward Roll"));
		}
		else if (RightDot > DiagonalThreshold)
		{
			Montage = RightMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Right Roll"));
		}
		else if (RightDot < -DiagonalThreshold)
		{
			Montage = LeftMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Left Roll"));
		}
		else
		{
			Montage = ForwardMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Defaulting to Forward Roll"));
		}
	}
	else
	{
		// 没有输入时，使用前向翻滚
		Montage = BackwardMontage;
		UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: No input, using Forward Roll"));
	}

	// 检查选中的动画是否有效
	if (!Montage)
	{
		
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Roll::SelectDirectionalMontage: Selected montage is null!"));
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

void UGA_Roll::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (CooldownDuration <= 0.0f)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC)
	{
		return;
	}

	UGameplayEffect* CooldownGE = NewObject<UGameplayEffect>(GetTransientPackage());
	CooldownGE->DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FGameplayEffectContextHandle EffectContext = MakeEffectContext(Handle, ActorInfo);
	float Level = GetAbilityLevel(Handle, ActorInfo);

	FGameplayEffectSpec* NewSpec = new FGameplayEffectSpec(CooldownGE, EffectContext, Level);
	FGameplayEffectSpecHandle SpecHandle(NewSpec);

	if (!SpecHandle.Data.IsValid())
	{
		return;
	}

	FGameplayEffectSpec& Spec = *SpecHandle.Data;
	Spec.SetDuration(CooldownDuration, true);
	Spec.DynamicGrantedTags.AppendTags(BlockCooldownTags);

	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

