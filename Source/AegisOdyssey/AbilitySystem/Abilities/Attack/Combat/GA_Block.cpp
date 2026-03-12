// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Block.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_Block)
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Animation/AOAnimInstance.h"
#include "AegisOdyssey/Animation/NotifyState/AOCombatWindow.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AegisOdyssey/AbilitySystem/Tasks/AT_WaitRotateToDirection.h"
#include "AegisOdyssey/AbilitySystem/Attributes/AOCombatAttributeSet.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetSystemLibrary.h"

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
	StartBlockMontage = nullptr;
	LoopBlockMontage = nullptr;
	EndBlockMontage = nullptr;
	RotationInterpSpeed = 360.0f;
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

	CommitAbility(Handle, ActorInfo, ActivationInfo);

	WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_Block::OnInputReleased);
		WaitInputReleaseTask->ReadyForActivation();
	}
	PlayStartBlockAnimation();
	UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::ActivateAbility: Called"));

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		UGameplayEffect* NewGE = NewObject<UGameplayEffect>(this);
		NewGE->DurationPolicy = EGameplayEffectDurationType::Infinite;
        
		FGameplayModifierInfo ModifierInfo;
		ModifierInfo.Attribute = UAOCombatAttributeSet::GetSprintSpeedBonusAttribute();
		ModifierInfo.ModifierOp = EGameplayModOp::Override;
		ModifierInfo.ModifierMagnitude = FScalableFloat(SprintSpeedBonusAmount);
        
		NewGE->Modifiers.Add(ModifierInfo);
        
		FGameplayEffectSpec Spec(NewGE, MakeEffectContext(Handle, ActorInfo));
		SprintSpeedEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(Spec);
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
	if (RotationTask)
	{
		RotationTask->EndTask();
		RotationTask = nullptr;
	}
	if (SprintSpeedEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(SprintSpeedEffectHandle, 1);
		}
		SprintSpeedEffectHandle.Invalidate();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::EndAbility: Called"));
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

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::SetCharacterRotationToBlockDirection: PlayerController is null"));
		return;
	}
	
	FRotator ControlRotation = PC->GetControlRotation();
	FRotator TargetRotation = FRotator(0.0f, ControlRotation.Yaw, 0.0f);
	
	FRotator CurrentRotation = Pawn->GetActorRotation();
	UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Block::SetCharacterRotationToBlockDirection: Current Yaw=%.2f, Target Yaw=%.2f, InterpSpeed=%.2f"), 
		CurrentRotation.Yaw, TargetRotation.Yaw, RotationInterpSpeed);

	if (RotationTask)
	{
		RotationTask->EndTask();
		RotationTask = nullptr;
	}
	
	RotationTask = UAT_WaitRotateToDirection::WaitRotateToDirection(this, TargetRotation, RotationInterpSpeed);
	if (RotationTask)
	{
		RotationTask->ReadyForActivation();
		UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Block::SetCharacterRotationToBlockDirection: Started rotation to Yaw: %.2f with speed: %.2f"), TargetRotation.Yaw, RotationInterpSpeed);
	}
	else
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_Block::SetCharacterRotationToBlockDirection: Failed to create RotationTask"));
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
	
	SetCharacterRotationToBlockDirection();
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
