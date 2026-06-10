// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_PlayAnimationMontage.h"

#include "AegisOdyssey/AOLogChannels.h"
#include "Animation/AnimInstance.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_PlayAnimationMontage)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Input_PlayEquipMontage, "Ability.Input.PlayEquipMontage");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Input_PlayUnEquipMontage, "Ability.Input.PlayUnEquipMontage");

UGA_PlayAnimationMontage::UGA_PlayAnimationMontage(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	Montage = nullptr;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FAbilityTriggerData PlayEquipMontageTrigger;
	PlayEquipMontageTrigger.TriggerTag = TAG_Ability_Input_PlayEquipMontage;
	PlayEquipMontageTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(PlayEquipMontageTrigger);

	FAbilityTriggerData PlayUnEquipMontageTrigger;
	PlayUnEquipMontageTrigger.TriggerTag = TAG_Ability_Input_PlayUnEquipMontage;
	PlayUnEquipMontageTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(PlayUnEquipMontageTrigger);
}

bool UGA_PlayAnimationMontage::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

void UGA_PlayAnimationMontage::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!TriggerEventData)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_PlayAnimationMontage::ActivateAbility: TriggerEventData is null."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TriggerEventData->TargetData.Data)
	{
		if (Data.IsValid() && Data->GetScriptStruct() == FPlayAnimationMontageTargetData::StaticStruct())
		{
			FPlayAnimationMontageTargetData* PlayMontageData = static_cast<FPlayAnimationMontageTargetData*>(Data.Get());

			Montage = PlayMontageData->DataMontage.Get();
		}
	}
	PlayMontageAnimation();
}

void UGA_PlayAnimationMontage::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (PlayMontageAndWaitTask)
	{
		PlayMontageAndWaitTask->EndTask();
		PlayMontageAndWaitTask = nullptr;
	}

	if (Montage)
	{
		Montage = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_PlayAnimationMontage::PlayMontageAnimation()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_PlayAnimationMontage::PlayMontageAnimation: Avatar=%s AnimInstance=%s Montage=%s"),
		*GetNameSafe(GetAvatarActorFromActorInfo()),
		*GetNameSafe(AnimInstance),
		*GetNameSafe(Montage));

	if (!Montage)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_PlayAnimationMontage::PlayMontageAnimation: Montage is null."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!AnimInstance)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_PlayAnimationMontage::PlayMontageAnimation: AnimInstance is null."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (PlayMontageAndWaitTask)
	{
		PlayMontageAndWaitTask->EndTask();
		PlayMontageAndWaitTask = nullptr;
	}

	PlayMontageAndWaitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("PlayMontageAndWait"),
		Montage,
		1.0f,
		FName("None"),
		true,
		1,
		0,
		false
		);

	if (!PlayMontageAndWaitTask)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_PlayAnimationMontage::PlayMontageAnimation: Failed to create PlayMontageAndWaitTask."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	PlayMontageAndWaitTask->OnBlendOut.AddDynamic(this, &UGA_PlayAnimationMontage::OnMontageBlendedOut);
	PlayMontageAndWaitTask->OnCompleted.AddDynamic(this, &UGA_PlayAnimationMontage::OnMontageCompleted);
	PlayMontageAndWaitTask->OnInterrupted.AddDynamic(this, &UGA_PlayAnimationMontage::OnMontageInterrupted);
	PlayMontageAndWaitTask->OnCancelled.AddDynamic(this, &UGA_PlayAnimationMontage::OnMontageCancelled);
	PlayMontageAndWaitTask->ReadyForActivation();
}

void UGA_PlayAnimationMontage::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_PlayAnimationMontage::OnMontageBlendedOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_PlayAnimationMontage::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_PlayAnimationMontage::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_PlayAnimationMontage::OnMovementInputDetected()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
