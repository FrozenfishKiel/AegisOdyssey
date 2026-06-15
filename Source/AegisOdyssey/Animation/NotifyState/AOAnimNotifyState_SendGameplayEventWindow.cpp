// Fill out your copyright notice in the Description page of Project Settings.

#include "AOAnimNotifyState_SendGameplayEventWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAnimNotifyState_SendGameplayEventWindow)

namespace
{
	void SendGameplayEventFromNotifyWindowAnimation(AActor* OwnerActor, UAnimSequenceBase* Animation, const FGameplayTag& EventTag, float EventMagnitude)
	{
		if (OwnerActor == nullptr || !EventTag.IsValid())
		{
			return;
		}

		FGameplayEventData Payload;
		Payload.EventTag = EventTag;
		Payload.Instigator = OwnerActor;
		Payload.Target = OwnerActor;
		Payload.OptionalObject = Animation;
		Payload.EventMagnitude = EventMagnitude;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
	}
}

UAOAnimNotifyState_SendGameplayEventWindow::UAOAnimNotifyState_SendGameplayEventWindow()
{
}

FString UAOAnimNotifyState_SendGameplayEventWindow::GetNotifyName_Implementation() const
{
	if (BeginEventTag.IsValid() && EndEventTag.IsValid())
	{
		return FString::Printf(TEXT("AO Window: %s -> %s"), *BeginEventTag.ToString(), *EndEventTag.ToString());
	}

	if (BeginEventTag.IsValid())
	{
		return FString::Printf(TEXT("AO Window Begin: %s"), *BeginEventTag.ToString());
	}

	if (EndEventTag.IsValid())
	{
		return FString::Printf(TEXT("AO Window End: %s"), *EndEventTag.ToString());
	}

	return TEXT("AO Gameplay Event Window");
}

void UAOAnimNotifyState_SendGameplayEventWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	SendGameplayEventFromNotifyWindowAnimation(MeshComp->GetOwner(), Animation, BeginEventTag, TotalDuration);
}

void UAOAnimNotifyState_SendGameplayEventWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	SendGameplayEventFromNotifyWindowAnimation(MeshComp->GetOwner(), Animation, EndEventTag, 0.0f);
}
