// Fill out your copyright notice in the Description page of Project Settings.

#include "AOAnimNotify_SendGameplayEvent.h"

#include "AegisOdyssey/AOLogChannels.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAnimNotify_SendGameplayEvent)

namespace
{
	void SendGameplayEventFromNotifyAnimation(AActor* OwnerActor, UAnimSequenceBase* Animation, const FGameplayTag& EventTag, float EventMagnitude)
	{
	if (OwnerActor == nullptr || !EventTag.IsValid())
	{
		return;
	}

	UE_LOG(
		LogAegisOdysseyAbilitySystem,
		Log,
		TEXT("UAOAnimNotify_SendGameplayEvent: Send event [%s] from owner [%s], animation [%s]."),
		*EventTag.ToString(),
		*GetNameSafe(OwnerActor),
		*GetNameSafe(Animation));

	FGameplayEventData Payload;
		Payload.EventTag = EventTag;
		Payload.Instigator = OwnerActor;
		Payload.Target = OwnerActor;
		Payload.OptionalObject = Animation;
		Payload.EventMagnitude = EventMagnitude;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
	}
}

UAOAnimNotify_SendGameplayEvent::UAOAnimNotify_SendGameplayEvent()
{
}

FString UAOAnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	return EventTag.IsValid() ? FString::Printf(TEXT("AO Event: %s"), *EventTag.ToString()) : TEXT("AO Event");
}

void UAOAnimNotify_SendGameplayEvent::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	SendGameplayEventFromNotifyAnimation(MeshComp->GetOwner(), Animation, EventTag, 0.0f);
}
