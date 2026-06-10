// Fill out your copyright notice in the Description page of Project Settings.

#include "GA_HitReact.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AegisOdyssey/AOCombatEventTags.h"
#include "AegisOdyssey/AOStateTags.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOCharacterCombatManagerComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_HitReact)

UGA_HitReact::UGA_HitReact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRetriggerInstancedAbility = true;

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = AOCombatEventTags::GameplayEvent_Combat_HitReact_Activate;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

bool UGA_HitReact::CanActivateAbility(
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

void UGA_HitReact::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	Montage = nullptr;

	if (TriggerEventData != nullptr)
	{
		for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TriggerEventData->TargetData.Data)
		{
			if (Data.IsValid() && Data->GetScriptStruct() == FHitReactTargetData::StaticStruct())
			{
				FHitReactTargetData* HitReactData = static_cast<FHitReactTargetData*>(Data.Get());
				Montage = HitReactData->Montage.LoadSynchronous();
				if (Montage == nullptr)
				{
					TSoftObjectPtr<UAnimMontage> ResolvedMontage;
					if (!TryResolveHitReactMontage(HitReactData->StateTag, HitReactData->SourceDirection, ResolvedMontage))
					{
						EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
						return;
					}

					Montage = ResolvedMontage.LoadSynchronous();
				}
				break;
			}
		}
	}

	if (Montage == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	WaitAllowMoveTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AOCombatEventTags::GameplayEvent_Combat_HitReact_AllowMove, nullptr, false, true);
	if (WaitAllowMoveTask != nullptr)
	{
		WaitAllowMoveTask->EventReceived.AddDynamic(this, &ThisClass::OnAllowMoveEvent);
		WaitAllowMoveTask->ReadyForActivation();
	}

	WaitFinishTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AOCombatEventTags::GameplayEvent_Combat_HitReact_Finish, nullptr, false, true);
	if (WaitFinishTask != nullptr)
	{
		WaitFinishTask->EventReceived.AddDynamic(this, &ThisClass::OnFinishEvent);
		WaitFinishTask->ReadyForActivation();
	}

	PlayMontageAnimation();
}

void UGA_HitReact::EndAbility(
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
			CombatManager->EndHitReactState();
		}
	}

	Montage = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_HitReact::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_HitReact::OnMontageBlendedOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_HitReact::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_HitReact::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_HitReact::OnAllowMoveEvent(FGameplayEventData Payload)
{
	ReleaseMoveLock();
}

void UGA_HitReact::OnFinishEvent(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_HitReact::PlayMontageAnimation()
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
		FName("HitReactMontage"),
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

EAOHitReactDirection UGA_HitReact::ResolveHitReactDirection(const FVector& SourceDirection) const
{
	const AAOCharacter* OwnerCharacter = GetLyraCharacterFromActorInfo();
	if (OwnerCharacter == nullptr)
	{
		return EAOHitReactDirection::Backward;
	}

	FVector ReactDirection = SourceDirection;
	ReactDirection.Z = 0.0f;
	if (ReactDirection.IsNearlyZero())
	{
		return EAOHitReactDirection::Backward;
	}

	ReactDirection.Normalize();

	const FVector OwnerForward = OwnerCharacter->GetActorForwardVector().GetSafeNormal2D();
	const FVector OwnerRight = OwnerCharacter->GetActorRightVector().GetSafeNormal2D();
	const float ForwardDot = FVector::DotProduct(ReactDirection, OwnerForward);
	const float RightDot = FVector::DotProduct(ReactDirection, OwnerRight);
	constexpr float DiagonalThreshold = 0.5f;

	if (ForwardDot > DiagonalThreshold && RightDot > DiagonalThreshold)
	{
		return EAOHitReactDirection::ForwardRight;
	}
	if (ForwardDot > DiagonalThreshold && RightDot < -DiagonalThreshold)
	{
		return EAOHitReactDirection::ForwardLeft;
	}
	if (ForwardDot < -DiagonalThreshold && RightDot > DiagonalThreshold)
	{
		return EAOHitReactDirection::BackwardRight;
	}
	if (ForwardDot < -DiagonalThreshold && RightDot < -DiagonalThreshold)
	{
		return EAOHitReactDirection::BackwardLeft;
	}
	if (ForwardDot > DiagonalThreshold)
	{
		return EAOHitReactDirection::Forward;
	}
	if (ForwardDot < -DiagonalThreshold)
	{
		return EAOHitReactDirection::Backward;
	}
	if (RightDot > DiagonalThreshold)
	{
		return EAOHitReactDirection::Right;
	}
	return EAOHitReactDirection::Left;
}

const TArray<TSoftObjectPtr<UAnimMontage>>* UGA_HitReact::FindMontagePool(
	const FAOHitReactMontagePoolSet& Pools,
	const EAOHitReactDirection Direction) const
{
	if (const FAOHitReactMontageList* PoolList = Pools.DirectionPools.Find(Direction))
	{
		return &PoolList->Montages;
	}

	return nullptr;
}

const FAOHitReactMontagePoolSet* UGA_HitReact::FindMontagePoolsByStateTag(const FGameplayTag& StateTag) const
{
	return MontagePoolsByStateTag.Find(StateTag);
}

bool UGA_HitReact::TryResolveHitReactMontage(
	const FGameplayTag& StateTag,
	const FVector& SourceDirection,
	TSoftObjectPtr<UAnimMontage>& OutMontage) const
{
	const FAOHitReactMontagePoolSet* PoolSet = FindMontagePoolsByStateTag(StateTag);
	if (PoolSet == nullptr)
	{
		return false;
	}

	const EAOHitReactDirection Direction = ResolveHitReactDirection(SourceDirection);
	const TArray<TSoftObjectPtr<UAnimMontage>>* Pool = FindMontagePool(*PoolSet, Direction);
	if (Pool == nullptr || Pool->Num() <= 0)
	{
		Pool = FindMontagePool(*PoolSet, EAOHitReactDirection::Backward);
	}

	if (Pool == nullptr || Pool->Num() <= 0)
	{
		return false;
	}

	OutMontage = (*Pool)[FMath::RandRange(0, Pool->Num() - 1)];
	return true;
}

void UGA_HitReact::ReleaseMoveLock() const
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
