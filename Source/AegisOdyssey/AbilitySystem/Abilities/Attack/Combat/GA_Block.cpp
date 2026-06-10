// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Block.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_Block)
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Animation/AOAnimInstance.h"
#include "AegisOdyssey/Animation/NotifyState/AOCombatWindow.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/AOCombatEventTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AegisOdyssey/AbilitySystem/Tasks/AT_WaitRotateToDirection.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Controller.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Block_Cooldown, "Ability.Block.Cooldown");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Block_Blocking, "Ability.Block.Blocking");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Block_Parry, "Ability.Block.Parry");

static const FGameplayTagContainer RollCooldownTags(TAG_Ability_Block_Cooldown);


UGA_Block::UGA_Block(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = EAOAbilityActivationPolicy::WhileInputActive;
	InputType = EInputType::None;
	Montage = nullptr;
	PlayRate = 0.f;
	StartTime = 0.f;
	Montage = nullptr;
	PlayEndBlockMontageTask = nullptr;
	PlayStartBlockMontageTask = nullptr;
	PlayLoopBlockMontageTask = nullptr;
	WaitInputReleaseTask = nullptr;
	WaitParrySuccessEventTask = nullptr;
	WaitFullBlockSuccessEventTask = nullptr;
	WaitPartialBlockSuccessEventTask = nullptr;
	StartBlockMontage = nullptr;
	LoopBlockMontage = nullptr;
	EndBlockMontage = nullptr;
	RotationInterpSpeed = 360.0f;
	CooldownDuration = 0.f;
}

bool UGA_Block::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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

	return true;}

void UGA_Block::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData)
	{
		for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TriggerEventData->TargetData.Data)
		{
			if (Data.IsValid() && Data->GetScriptStruct() == FBlockTargetData::StaticStruct())
			{
				FBlockTargetData* BlockTargetData = static_cast<FBlockTargetData*>(Data.Get());
				
				InputTag = BlockTargetData->InputTag;
				PlayRate = BlockTargetData->PlayRate;
				StartSection = BlockTargetData->StartSection;
				StartTime = BlockTargetData->StartTime;
				
				StartBlockMontage = BlockTargetData->StartBlockMontage.Get();
				LoopBlockMontage = BlockTargetData->LoopBlockMontage.Get();
				EndBlockMontage = BlockTargetData->EndBlockMontage.Get();
				
				break;
			}
		}
	}

	SetCharacterRotationToBlockDirection();

	WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_Block::OnInputReleased);
		WaitInputReleaseTask->ReadyForActivation();
	}

	WaitParrySuccessEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AOCombatEventTags::GameplayEvent_Combat_Block_ParrySuccess, nullptr, false, true);
	if (WaitParrySuccessEventTask)
	{
		WaitParrySuccessEventTask->EventReceived.AddDynamic(this, &ThisClass::OnParrySuccessEvent);
		WaitParrySuccessEventTask->ReadyForActivation();
	}

	WaitFullBlockSuccessEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AOCombatEventTags::GameplayEvent_Combat_Block_FullBlockSuccess, nullptr, false, true);
	if (WaitFullBlockSuccessEventTask)
	{
		WaitFullBlockSuccessEventTask->EventReceived.AddDynamic(this, &ThisClass::OnFullBlockSuccessEvent);
		WaitFullBlockSuccessEventTask->ReadyForActivation();
	}

	WaitPartialBlockSuccessEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AOCombatEventTags::GameplayEvent_Combat_Block_PartialBlockSuccess, nullptr, false, true);
	if (WaitPartialBlockSuccessEventTask)
	{
		WaitPartialBlockSuccessEventTask->EventReceived.AddDynamic(this, &ThisClass::OnPartialBlockSuccessEvent);
		WaitPartialBlockSuccessEventTask->ReadyForActivation();
	}

	PlayStartBlockAnimation();
	UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::ActivateAbility: Called"));

	if (BlockActiveEffectClass != nullptr)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(BlockActiveEffectClass, GetAbilityLevel(Handle, ActorInfo));
			if (SpecHandle.IsValid())
			{
				BlockActiveEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

}

void UGA_Block::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearCombatTags();
	
	if (PlayStartBlockMontageTask)
	{
		PlayStartBlockMontageTask->EndTask();
		PlayStartBlockMontageTask = nullptr;
	}
	
	if (PlayLoopBlockMontageTask)
	{
		PlayLoopBlockMontageTask->EndTask();
		PlayLoopBlockMontageTask = nullptr;
	}
	
	if (PlayEndBlockMontageTask)
	{
		PlayEndBlockMontageTask->EndTask();
		PlayEndBlockMontageTask = nullptr;
	}
	
	if (WaitGameplayEventTask)
	{
		WaitGameplayEventTask->EndTask();
		WaitGameplayEventTask = nullptr;
	}
	
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->EndTask();
		WaitInputReleaseTask = nullptr;
	}
	if (WaitParrySuccessEventTask)
	{
		WaitParrySuccessEventTask->EndTask();
		WaitParrySuccessEventTask = nullptr;
	}
	if (WaitFullBlockSuccessEventTask)
	{
		WaitFullBlockSuccessEventTask->EndTask();
		WaitFullBlockSuccessEventTask = nullptr;
	}
	if (WaitPartialBlockSuccessEventTask)
	{
		WaitPartialBlockSuccessEventTask->EndTask();
		WaitPartialBlockSuccessEventTask = nullptr;
	}
	if (RotationTask)
	{
		RotationTask->EndTask();
		RotationTask = nullptr;
	}
	if (BlockActiveEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(BlockActiveEffectHandle, 1);
		}
		BlockActiveEffectHandle.Invalidate();
	}
	if (bWasCancelled)  //如果是被Cancel的能力，则应用冷却
	{
		CommitAbility(Handle, ActorInfo, ActivationInfo);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::EndAbility: Called"));
}

void UGA_Block::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
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
	
	
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (!CooldownGE)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::ApplyCooldown: CooldownGameplayEffectClass is not set in blueprint!"));
		return;
	}


	FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(Handle);
	if (AbilitySpec)
	{
		AbilitySpec->SetByCallerTagMagnitudes.FindOrAdd(TAG_Ability_Block_Cooldown) = CooldownDuration;
	}
	FGameplayEffectContextHandle EffectContext = MakeEffectContext(Handle, ActorInfo);
	float Level = GetAbilityLevel(Handle, ActorInfo);
	// 创建GE Spec
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), Level);
	if (!SpecHandle.Data.IsValid())
	{
		return;
	}

	Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
	
}

const FGameplayTagContainer* UGA_Block::GetCooldownTags() const
{
	return &RollCooldownTags;
}

void UGA_Block::OnMontageCompleted()
{
	if (PlayStartBlockMontageTask)
	{
		PlayLoopBlockAnimation();
	}
	else if (PlayLoopBlockMontageTask)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else if (PlayEndBlockMontageTask)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_Block::OnMontageBlendedOut()
{
	//EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Block::OnMontageInterrupted()
{
	//EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Block::OnMontageCancelled()
{
	//EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Block::OnInputReleased(float TimeHeld)
{
	UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Block::OnInputReleased: Input released, TimeHeld=%.2f"), TimeHeld);
	PlayEndBlockAnimation();
}

void UGA_Block::OnParrySuccessEvent(FGameplayEventData Payload)
{
	ApplyEffectToSelf(ParrySuccessEffectClass);
}

void UGA_Block::OnFullBlockSuccessEvent(FGameplayEventData Payload)
{
	ApplyEffectToSelf(FullBlockSuccessEffectClass);
}

void UGA_Block::OnPartialBlockSuccessEvent(FGameplayEventData Payload)
{
	ApplyEffectToSelf(PartialBlockSuccessEffectClass);
}

void UGA_Block::ClearCombatTags()
{
	AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
	if (!AOCharacter)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::ClearCombatTags: AOCharacter is null"));
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AOCharacter);
	if (!ASC)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::ClearCombatTags: ASC is null"));
		return;
	}

	TArray<FGameplayTag> CombatTags;
	if (StartBlockMontage)
	{
		GetCombatWindowTagsFromMontage(StartBlockMontage, CombatTags);
	}
	if (LoopBlockMontage)
	{
		GetCombatWindowTagsFromMontage(LoopBlockMontage, CombatTags);
	}
	if (EndBlockMontage)
	{
		GetCombatWindowTagsFromMontage(EndBlockMontage, CombatTags);
	}

	for (const FGameplayTag& Tag : CombatTags)
	{
		if (Tag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(Tag);
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Block::ClearCombatTags: Removed tag: %s"), *Tag.ToString());
		}
	}
}

void UGA_Block::GetCombatWindowTagsFromMontage(UAnimMontage* InMontage, TArray<FGameplayTag>& OutTags)
{
	if (!InMontage)
	{
		return;
	}

	for (const FAnimNotifyEvent& NotifyEvent : InMontage->Notifies)
	{
		if (NotifyEvent.NotifyStateClass && NotifyEvent.NotifyStateClass->IsA<UAOCombatWindow>())
		{
			UAOCombatWindow* CombatWindowCDO = Cast<UAOCombatWindow>(NotifyEvent.NotifyStateClass);
			if (!CombatWindowCDO)
			{
				continue;
			}
			
			FGameplayTag CombatWindowTag = CombatWindowCDO->GetCombatWindowTag();
			if (CombatWindowTag.IsValid())
			{
				OutTags.AddUnique(CombatWindowTag);
				UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Block::GetCombatWindowTagsFromMontage: Found CombatWindowTag: %s"), *CombatWindowTag.ToString());
			}
			
			FGameplayTag CombatingTag = CombatWindowCDO->GetCombatingTag();
			if (CombatingTag.IsValid())
			{
				OutTags.AddUnique(CombatingTag);
				UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Block::GetCombatWindowTagsFromMontage: Found CombatingTag: %s"), *CombatingTag.ToString());
			}
		}
	}
}

void UGA_Block::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass) const
{
	if (EffectClass == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC == nullptr)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectClass, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UGA_Block::SetCharacterRotationToBlockDirection()
{
	if (!CurrentActorInfo)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::SetCharacterRotationToBlockDirection: CurrentActorInfo is null"));
		return;
	}

	APawn* Pawn = Cast<APawn>(CurrentActorInfo->AvatarActor);
	if (!Pawn)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::SetCharacterRotationToBlockDirection: Pawn is null"));
		return;
	}

	AController* Controller = Pawn->GetController();
	if (!Controller)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::SetCharacterRotationToBlockDirection: Controller is null"));
		return;
	}
	
	FRotator ControlRotation = Controller->GetControlRotation();
	FRotator TargetRotation = FRotator(0.0f, ControlRotation.Yaw, 0.0f);
	
	if (RotationTask)
	{
		RotationTask->EndTask();
		RotationTask = nullptr;
	}
	
	RotationTask = UAT_WaitRotateToDirection::WaitRotateToDirection(this, TargetRotation, RotationInterpSpeed, true);
	if (RotationTask)
	{
		RotationTask->ReadyForActivation();
		UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Block::SetCharacterRotationToBlockDirection: Started continuous rotation to Yaw: %.2f with speed: %.2f"), TargetRotation.Yaw, RotationInterpSpeed);
	}
}


void UGA_Block::PlayStartBlockAnimation()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Block::PlayStartBlockAnimation: AnimInstance: %s"), 
		*GetNameSafe(AnimInstance));
    
	if (!AnimInstance)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_Block::PlayStartBlockAnimation: AnimInstance is null!"));
		return;
	}

	if (!StartBlockMontage)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_Block::PlayStartBlockAnimation: StartBlockMontage is null!"));
		PlayLoopBlockAnimation();  //如果没有启动动画就跳过启动直接进入循环
		return;
	}

	if (PlayStartBlockMontageTask)
	{
		PlayStartBlockMontageTask->EndTask();
		PlayStartBlockMontageTask = nullptr;
	}

	PlayStartBlockMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("PlayMontageAndWait"),
		StartBlockMontage,
		PlayRate,
		StartSection,
		true,
		1.0f,
		StartTime,
		false
	);

	if (PlayStartBlockMontageTask)
	{
		PlayStartBlockMontageTask->OnBlendOut.AddDynamic(this, &UGA_Block::OnMontageBlendedOut);
		PlayStartBlockMontageTask->OnCompleted.AddDynamic(this, &UGA_Block::OnMontageCompleted);
		PlayStartBlockMontageTask->OnInterrupted.AddDynamic(this, &UGA_Block::OnMontageInterrupted);
		PlayStartBlockMontageTask->OnCancelled.AddDynamic(this, &UGA_Block::OnMontageCancelled);
		PlayStartBlockMontageTask->ReadyForActivation();
	}
}

void UGA_Block::PlayLoopBlockAnimation()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Block::PlayLoopBlockAnimation: AnimInstance: %s"), 
		*GetNameSafe(AnimInstance));
    
	if (!AnimInstance)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_Block::PlayLoopBlockAnimation: AnimInstance is null!"));
		return;
	}

	if (!LoopBlockMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (PlayLoopBlockMontageTask)
	{
		PlayLoopBlockMontageTask->EndTask();
		PlayLoopBlockMontageTask = nullptr;
	}

	PlayLoopBlockMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("PlayMontageAndWait"),
		LoopBlockMontage,
		PlayRate,
		StartSection,
		true,
		1.0f,
		StartTime,
		false
	);

	if (PlayLoopBlockMontageTask)
	{
		PlayLoopBlockMontageTask->OnBlendOut.AddDynamic(this, &UGA_Block::OnMontageBlendedOut);
		PlayLoopBlockMontageTask->OnCompleted.AddDynamic(this, &UGA_Block::OnMontageCompleted);
		PlayLoopBlockMontageTask->OnInterrupted.AddDynamic(this, &UGA_Block::OnMontageInterrupted);
		PlayLoopBlockMontageTask->OnCancelled.AddDynamic(this, &UGA_Block::OnMontageCancelled);
		PlayLoopBlockMontageTask->ReadyForActivation();
	}
	
}

void UGA_Block::PlayEndBlockAnimation()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Block::PlayEndBlockAnimation: AnimInstance: %s"), 
		*GetNameSafe(AnimInstance));
    
	if (!AnimInstance)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_Block::PlayEndBlockAnimation: AnimInstance is null!"));
		return;
	}

	if (!EndBlockMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	if (PlayLoopBlockMontageTask)
	{
		PlayLoopBlockMontageTask->EndTask();
		PlayLoopBlockMontageTask = nullptr;
	}

	//清理完其他两个动画
	if (PlayStartBlockMontageTask)
	{
		PlayStartBlockMontageTask->EndTask();
		PlayStartBlockMontageTask = nullptr;
	}

	if (PlayEndBlockMontageTask)
	{
		PlayEndBlockMontageTask->EndTask();
		PlayEndBlockMontageTask = nullptr;  //确保取消蒙太奇动画的Task清理干净
	}

	PlayEndBlockMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("PlayMontageAndWait"),
		EndBlockMontage,
		PlayRate,
		StartSection,
		true,
		1.0f,
		StartTime,
		false
	);

	if (PlayEndBlockMontageTask)
	{
		PlayEndBlockMontageTask->OnBlendOut.AddDynamic(this, &UGA_Block::OnMontageBlendedOut);
		PlayEndBlockMontageTask->OnCompleted.AddDynamic(this, &UGA_Block::OnMontageCompleted);
		PlayEndBlockMontageTask->OnInterrupted.AddDynamic(this, &UGA_Block::OnMontageInterrupted);
		PlayEndBlockMontageTask->OnCancelled.AddDynamic(this, &UGA_Block::OnMontageCancelled);
		PlayEndBlockMontageTask->ReadyForActivation();
	}
}
