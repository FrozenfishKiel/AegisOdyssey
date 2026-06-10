// Fill out your copyright notice in the Description page of Project Settings.

#include "STC_ActorHasMatchTag.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(STC_ActorHasMatchTag)

namespace
{
UAbilitySystemComponent* ResolveAbilitySystemComponentForTagCheck(AActor* TargetActor)
{
	if (TargetActor == nullptr)
	{
		return nullptr;
	}

	if (APlayerState* PlayerState = Cast<APlayerState>(TargetActor))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState))
		{
			return AbilitySystemComponent;
		}
	}

	if (APawn* Pawn = Cast<APawn>(TargetActor))
	{
		if (APlayerState* PlayerState = Pawn->GetPlayerState())
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState))
			{
				return AbilitySystemComponent;
			}
		}
	}

	if (AController* Controller = Cast<AController>(TargetActor))
	{
		if (APlayerState* PlayerState = Controller->PlayerState)
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState))
			{
				return AbilitySystemComponent;
			}
		}

		if (APawn* Pawn = Controller->GetPawn())
		{
			if (const UAOExtPawnComponent* PawnExtComponent = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
			{
				if (UAbilitySystemComponent* AbilitySystemComponent = PawnExtComponent->GetAbilitySystemComponent())
				{
					return AbilitySystemComponent;
				}
			}
		}
	}

	if (const UAOExtPawnComponent* PawnExtComponent = UAOExtPawnComponent::FindAOExtPawnComponent(TargetActor))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = PawnExtComponent->GetAbilitySystemComponent())
		{
			return AbilitySystemComponent;
		}
	}

	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
}
}

bool FSTC_ActorHasMatchTag::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FActorHasMatchTagInstanceData& InstanceData = Context.GetInstanceData<FActorHasMatchTagInstanceData>(*this);
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponentForTagCheck(OwnerActor);
	const bool bHasTag = AbilitySystemComponent != nullptr && AbilitySystemComponent->HasMatchingGameplayTag(InstanceData.InTag);
	return bHasTag ^ bInvert;
}
