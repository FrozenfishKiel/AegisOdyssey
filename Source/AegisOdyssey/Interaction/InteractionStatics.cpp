// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionStatics.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionStatics)

UInteractionStatics::UInteractionStatics()
{
	
}

AActor* UInteractionStatics::GetActorFromInteractableTarget(TScriptInterface<IInteractableTarget> InteractableTarget)
{
	if (UObject* Object = InteractableTarget.GetObject())
	{
		if (AActor* ActorRef = Cast<AActor>(Object))
		{
			return ActorRef;
		}
		else if (UActorComponent* ActorComp = Cast<UActorComponent>(Object))
		{
			return ActorComp->GetOwner();
		}
		else
		{
			unimplemented();
		}
	}
	return nullptr;
}

void UInteractionStatics::AppendInteractableTargetsFromHitResult(const FHitResult& HitResult,
                                                                 TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets)
{
	//检查某个Actor是否实现了模板接口，是则该变量有效，反之
	TScriptInterface<IInteractableTarget> InteractableActor(HitResult.GetActor());
	if (InteractableActor)
	{
		OutInteractableTargets.AddUnique(InteractableActor);
	}

	TScriptInterface<IInteractableTarget> InteractableComponent(HitResult.GetComponent());
	if (InteractableComponent)
	{
		OutInteractableTargets.AddUnique(InteractableComponent);
	}
}
