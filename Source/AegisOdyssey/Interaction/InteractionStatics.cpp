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

		if (UActorComponent* ActorComp = Cast<UActorComponent>(Object))
		{
			return ActorComp->GetOwner();
		}

		unimplemented();
	}

	return nullptr;
}

FName UInteractionStatics::GetComponentNameFromInteractableTarget(TScriptInterface<IInteractableTarget> InteractableTarget)
{
	if (UObject* Object = InteractableTarget.GetObject())
	{
		if (UActorComponent* ActorComp = Cast<UActorComponent>(Object))
		{
			return ActorComp->GetFName();
		}
	}

	return NAME_None;
}

TScriptInterface<IInteractableTarget> UInteractionStatics::ResolveInteractableTarget(AActor* TargetActor, FName TargetComponentName)
{
	if (!TargetActor)
	{
		return TScriptInterface<IInteractableTarget>();
	}

	// 优先按组件名恢复组件侧交互目标，保证统一交互链未来也能覆盖组件型交互对象。
	if (TargetComponentName != NAME_None)
	{
		TArray<UActorComponent*> ActorComponents;
		TargetActor->GetComponents(ActorComponents);
		for (UActorComponent* ActorComponent : ActorComponents)
		{
			if (!ActorComponent || ActorComponent->GetFName() != TargetComponentName)
			{
				continue;
			}

			TScriptInterface<IInteractableTarget> InteractableComponent(ActorComponent);
			if (InteractableComponent)
			{
				return InteractableComponent;
			}
		}
	}

	TScriptInterface<IInteractableTarget> InteractableActor(TargetActor);
	return InteractableActor;
}

void UInteractionStatics::AppendInteractableTargetsFromHitResult(const FHitResult& HitResult,
	TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets)
{
	// 先尝试把命中的 Actor 自身当成交互目标。
	TScriptInterface<IInteractableTarget> InteractableActor(HitResult.GetActor());
	if (InteractableActor)
	{
		OutInteractableTargets.AddUnique(InteractableActor);
	}

	// 再尝试把命中的具体组件当成交互目标。
	TScriptInterface<IInteractableTarget> InteractableComponent(HitResult.GetComponent());
	if (InteractableComponent)
	{
		OutInteractableTargets.AddUnique(InteractableComponent);
	}
}

bool UInteractionStatics::TryExecuteInteraction(TScriptInterface<IInteractableTarget> InteractableTarget,
	const FGameplayTag& InteractionEventTag, FGameplayEventData& Payload)
{
	if (!InteractableTarget)
	{
		return false;
	}

	// 先允许目标补充上下文，再决定是否可以执行。
	InteractableTarget->CustomizeInteractionEventData(InteractionEventTag, Payload);

	if (!InteractableTarget->CanExecuteInteraction(InteractionEventTag, Payload))
	{
		return false;
	}

	return InteractableTarget->ExecuteInteraction(InteractionEventTag, Payload);
}

int32 UInteractionStatics::GetInteractionOptionIndexFromEventData(const FGameplayEventData& Payload)
{
	return FMath::Max(FMath::RoundToInt(Payload.EventMagnitude), 0);
}
