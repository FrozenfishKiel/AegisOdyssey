// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_LightAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "AegisOdyssey/StateTree/CombatStateTree/AOCombatStateTree.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Animation/AOAnimInstance.h"
#include "AegisOdyssey/Animation/NotifyState/AOCombatWindow.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AegisOdyssey/AbilitySystem/Tasks/AT_WaitMovementInput.h"
#include "AegisOdyssey/AbilitySystem/Tasks/AT_WaitRotateToDirection.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_LightAttack)

UGA_LightAttack::UGA_LightAttack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	InputType = EInputType::None;
	Montage = nullptr;
	PlayRate = 0.f;
	StartTime = 0.f;
	RotationInterpSpeed = 360.0f;
}

bool UGA_LightAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                         const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
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

void UGA_LightAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	/**
	 * 从TargetData获取参数（支持网络自动复制）
	 * FGameplayAbilityTargetData会自动复制到服务器，客户端和服务器都能获取到相同的数据
	 */
	if (TriggerEventData)
	{
		for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TriggerEventData->TargetData.Data)
		{
			if (Data.IsValid() && Data->GetScriptStruct() == FLightAttackTargetData::StaticStruct())
			{
				FLightAttackTargetData* LightAttackData = static_cast<FLightAttackTargetData*>(Data.Get());
				
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

	if (!MovementInputTask)
	{
		MovementInputTask = UAT_WaitMovementInput::WaitMovementInput(this);
		if (MovementInputTask)
		{
			MovementInputTask->OnMovementInputDetected.AddDynamic(this, &UGA_LightAttack::OnMovementInputDetected);
			MovementInputTask->ReadyForActivation();
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::OnRecoveryTagChanged: Started movement input detection"));
		}
	}
	else
	{
		if (MovementInputTask)
		{
			MovementInputTask->EndTask();
			MovementInputTask = nullptr;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::OnRecoveryTagChanged: Stopped movement input detection"));
		}
	}

	SetCharacterRotationToAttackDirection();

	PlayMontageAnimation();
}

void UGA_LightAttack::PlayMontageAnimation()
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
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_LightAttack::OnMontageBlendedOut);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_LightAttack::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_LightAttack::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_LightAttack::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
}

void UGA_LightAttack::OnMovementInputDetected()
{
	if (!CancelAbilityTag.IsValid()) return;
	if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(CancelAbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);  //强制和通知服务器结束能力
	}
}

void UGA_LightAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_LightAttack::OnMontageBlendedOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_LightAttack::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_LightAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_LightAttack::ClearCombatTags()
{
	AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
	if (!AOCharacter)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_LightAttack::ClearCombatTags: AOCharacter is null"));
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AOCharacter);
	if (!ASC)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_LightAttack::ClearCombatTags: ASC is null"));
		return;
	}

	// 获取蒙太奇中所有的CombatWindowTag
	TArray<FGameplayTag> CombatTags;
	if (Montage)
	{
		GetCombatWindowTagsFromMontage(Montage, CombatTags);
	}

	// 立即移除所有连招窗口标签
	for (const FGameplayTag& Tag : CombatTags)
	{
		if (Tag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(Tag);
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::ClearCombatTags: Removed tag: %s"), *Tag.ToString());
		}
	}
}

void UGA_LightAttack::GetCombatWindowTagsFromMontage(UAnimMontage* InMontage, TArray<FGameplayTag>& OutTags)
{
	if (!InMontage)
	{
		return;
	}

	// 遍历蒙太奇中的所有AnimNotify
	for (const FAnimNotifyEvent& NotifyEvent : InMontage->Notifies)
	{
		// 检查是否是UAOCombatWindow类型的AnimNotifyState
		if (NotifyEvent.NotifyStateClass && NotifyEvent.NotifyStateClass->IsA<UAOCombatWindow>())
		{
			// NotifyEvent.NotifyStateClass指向的是CDO（Class Default Object）
			// Tag的值是在编辑器中配置的，存储在CDO中
			UAOCombatWindow* CombatWindowCDO = Cast<UAOCombatWindow>(NotifyEvent.NotifyStateClass);
			if (!CombatWindowCDO)
			{
				continue;
			}
			
			// 直接使用Getter方法获取Tag值
			FGameplayTag CombatWindowTag = CombatWindowCDO->GetCombatWindowTag();
			if (CombatWindowTag.IsValid())
			{
				OutTags.AddUnique(CombatWindowTag);
				UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::GetCombatWindowTagsFromMontage: Found CombatWindowTag: %s"), *CombatWindowTag.ToString());
			}
			
			FGameplayTag CombatingTag = CombatWindowCDO->GetCombatingTag();
			if (CombatingTag.IsValid())
			{
				OutTags.AddUnique(CombatingTag);
				UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::GetCombatWindowTagsFromMontage: Found CombatingTag: %s"), *CombatingTag.ToString());
			}
		}
	}
}

void UGA_LightAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearCombatTags();
	if (MovementInputTask)
	{
		MovementInputTask->EndTask();
		MovementInputTask = nullptr;
	}
	
	if (RotationTask)
	{
		RotationTask->EndTask();
		RotationTask = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_LightAttack::EndAbility: Called"));
}

void UGA_LightAttack::SetCharacterRotationToAttackDirection()
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
	
	FRotator ControlRotation = PC->GetControlRotation();
	FRotator TargetRotation = FRotator(0.0f, ControlRotation.Yaw, 0.0f);

	if (!RotationTask)
	{
		RotationTask = UAT_WaitRotateToDirection::WaitRotateToDirection(this, TargetRotation, RotationInterpSpeed);
		if (RotationTask)
		{
			RotationTask->ReadyForActivation();
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::SetCharacterRotationToAttackDirection: Started rotation to Yaw: %.2f with speed: %.2f"), TargetRotation.Yaw, RotationInterpSpeed);
		}
	}
}
