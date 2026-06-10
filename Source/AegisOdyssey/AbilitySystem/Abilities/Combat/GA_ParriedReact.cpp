// Fill out your copyright notice in the Description page of Project Settings.

#include "GA_ParriedReact.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AegisOdyssey/AOCombatEventTags.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOCharacterCombatManagerComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_ParriedReact)

UGA_ParriedReact::UGA_ParriedReact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRetriggerInstancedAbility = true;

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = AOCombatEventTags::GameplayEvent_Combat_ParriedReact_Activate;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

bool UGA_ParriedReact::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return ActorInfo != nullptr && ActorInfo->AvatarActor.IsValid();
}

void UGA_ParriedReact::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	Montage = ReactMontage.LoadSynchronous();
	if (Montage == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ApplyActivateEffectToSelf();

	WaitAllowMoveTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AOCombatEventTags::GameplayEvent_Combat_ParriedReact_AllowMove, nullptr, false, true);
	if (WaitAllowMoveTask != nullptr)
	{
		WaitAllowMoveTask->EventReceived.AddDynamic(this, &ThisClass::OnAllowMoveEvent);
		WaitAllowMoveTask->ReadyForActivation();
	}

	WaitFinishTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AOCombatEventTags::GameplayEvent_Combat_ParriedReact_Finish, nullptr, false, true);
	if (WaitFinishTask != nullptr)
	{
		WaitFinishTask->EventReceived.AddDynamic(this, &ThisClass::OnFinishEvent);
		WaitFinishTask->ReadyForActivation();
	}

	PlayMontageAnimation();
}

void UGA_ParriedReact::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (MontageTask != nullptr)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	if (WaitAllowMoveTask != nullptr)
	{
		WaitAllowMoveTask->EndTask();
		WaitAllowMoveTask = nullptr;
	}

	if (WaitFinishTask != nullptr)
	{
		WaitFinishTask->EndTask();
		WaitFinishTask = nullptr;
	}

	if (AAOCharacter* OwnerCharacter = GetLyraCharacterFromActorInfo())
	{
		if (UAOCharacterCombatManagerComponent* CombatManager = OwnerCharacter->FindComponentByClass<UAOCharacterCombatManagerComponent>())
		{
			CombatManager->EndParriedReaction();
		}
	}

	Montage = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_ParriedReact::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_ParriedReact::OnMontageBlendedOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_ParriedReact::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_ParriedReact::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_ParriedReact::OnAllowMoveEvent(FGameplayEventData Payload)
{
	ReleaseMoveLock();
}

void UGA_ParriedReact::OnFinishEvent(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_ParriedReact::PlayMontageAnimation()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo != nullptr ? ActorInfo->GetAnimInstance() : nullptr;
	if (AnimInstance == nullptr || Montage == nullptr)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (MontageTask != nullptr)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("ParriedReactMontage"),
		Montage,
		1.0f,
		NAME_None,
		true,
		1.0f,
		0.0f,
		false);

	if (MontageTask == nullptr)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageBlendedOut);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_ParriedReact::ApplyActivateEffectToSelf()
{
	if (ActivateEffectClass == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC == nullptr)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ActivateEffectClass, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UGA_ParriedReact::ReleaseMoveLock() const
{
	if (AAOCharacter* OwnerCharacter = GetLyraCharacterFromActorInfo())
	{
		if (AController* Controller = OwnerCharacter->GetController())
		{
			Controller->SetIgnoreMoveInput(false);
		}

		if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
		{
			if (MovementComponent->MovementMode == MOVE_None)
			{
				MovementComponent->SetMovementMode(MOVE_Walking);
			}
		}
	}
}
