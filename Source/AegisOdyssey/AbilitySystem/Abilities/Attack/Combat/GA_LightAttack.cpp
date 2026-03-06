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

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_LightAttack)

UGA_LightAttack::UGA_LightAttack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	Params = nullptr;
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
				Params = NewObject<ULightAttackParams>(this);
				Params->InputTag = LightAttackData->InputTag;
				Params->InputType = LightAttackData->InputType;
				Params->Montage = LightAttackData->Montage.Get();
				Params->PlayRate = LightAttackData->PlayRate;
				Params->StartSection = LightAttackData->StartSection;
				Params->StartTime = LightAttackData->StartTime;
				
				UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_LightAttack::ActivateAbility: Got params from TargetData - InputTag: %s, Montage: %s, PlayRate: %.2f, Role: %s"), 
					*Params->InputTag.ToString(), 
					*GetNameSafe(Params->Montage), 
					Params->PlayRate,
					HasAuthority(&ActivationInfo) ? TEXT("Server") : TEXT("Client"));
				
				break;
			}
		}
	}

	if (!Params)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_LightAttack::ActivateAbility: No params found, ending ability"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

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
	
	UAnimMontage* MontageToPlay = Params ? Params->Montage : nullptr;
	float PlayRateValue = Params ? Params->PlayRate : 1.0f;
	FName StartSectionValue = Params ? Params->StartSection : NAME_None;
	float StartTimeValue = Params ? Params->StartTime : 0.0f;

	UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::PlayMontageAnimation: MontageToPlay: %s, PlayRate: %.2f, StartSection: %s, StartTime: %.2f"),
		MontageToPlay ? *MontageToPlay->GetName() : TEXT("None"),
		PlayRateValue,
		*StartSectionValue.ToString(),
		StartTimeValue);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("PlayMontageAndWait"),
		MontageToPlay,
		PlayRateValue,
		StartSectionValue,
		true,
		1.0f,
		StartTimeValue,
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
	if (Params && Params->Montage)
	{
		GetCombatWindowTagsFromMontage(Params->Montage, CombatTags);
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

void UGA_LightAttack::GetCombatWindowTagsFromMontage(UAnimMontage* Montage, TArray<FGameplayTag>& OutTags)
{
	if (!Montage)
	{
		return;
	}

	// 遍历蒙太奇中的所有AnimNotify
	for (const FAnimNotifyEvent& NotifyEvent : Montage->Notifies)
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
	// 在技能结束时立即清空标签，而不是等待AnimNotifyState的NotifyEnd()被调用
	ClearCombatTags();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_LightAttack::EndAbility: Called"));
}
