// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Jump.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "Abilities/Tasks/AbilityTask_WaitMovementModeChange.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_Jump)

UGA_Jump::UGA_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

bool UGA_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
	return (AOCharacter && AOCharacter->CanJump());
}

void UGA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AAOCharacter* AOCharacter = Cast<AAOCharacter>(AvatarActor))
	{
		UCharacterMovementComponent* MovementComp = AOCharacter->GetCharacterMovement();
		if (MovementComp && !MovementComp->IsFalling())
		{
			AOCharacter->Jump();
		}
	}
	
	UAbilityTask_WaitMovementModeChange* MovementModeTask = UAbilityTask_WaitMovementModeChange::CreateWaitMovementModeChange(this, MOVE_Walking);
	MovementModeTask->OnChange.AddDynamic(this, &UGA_Jump::OnMovementModeChanged);
	MovementModeTask->ReadyForActivation();
	
	UAbilityTask_WaitDelay* TimeoutTask = UAbilityTask_WaitDelay::WaitDelay(this, 5.0f);
	TimeoutTask->OnFinish.AddDynamic(this, &UGA_Jump::OnJumpTimeout);
	TimeoutTask->ReadyForActivation();
}

void UGA_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Jump::OnMovementModeChanged(EMovementMode NewMovementMode)
{
	if (NewMovementMode == MOVE_Walking)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_Jump::OnJumpTimeout()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Jump::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
	
	if (AAOCharacter* AOCharacter = Cast<AAOCharacter>(ActorInfo->AvatarActor.Get()))
	{
		AOCharacter->StopJumping();
	}
}
