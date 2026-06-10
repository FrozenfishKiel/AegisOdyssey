// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/Inventory/AOInventoryStatics.h"

#include "Algo/Sort.h"
#include "AegisOdyssey/Equipment/AOQuickBarComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/InventoryInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryStatics)

UAOInventoryComponent* UAOInventoryStatics::FindPreferredQuickBarInventoryComponent(
	const AActor* TargetActor,
	const TArray<UAOInventoryComponent*>& InventoryComponents)
{
	if (TargetActor == nullptr)
	{
		return nullptr;
	}

	UAOQuickBarComponent* QuickBarComponent = TargetActor->FindComponentByClass<UAOQuickBarComponent>();
	if (QuickBarComponent == nullptr)
	{
		return nullptr;
	}

	for (UAOInventoryComponent* InventoryComponent : InventoryComponents)
	{
		if (InventoryComponent == QuickBarComponent)
		{
			return QuickBarComponent;
		}
	}

	return nullptr;
}

void UAOInventoryStatics::CollectInventoryComponentsFromActor(const AActor* TargetActor, TArray<UAOInventoryComponent*>& OutInventoryComponents)
{
	OutInventoryComponents.Reset();

	if (TargetActor == nullptr)
	{
		return;
	}

	const TArray<UActorComponent*> InventoryComponents = TargetActor->GetComponentsByInterface(UInventoryInterface::StaticClass());
	for (UActorComponent* ActorComponent : InventoryComponents)
	{
		TScriptInterface<IInventoryInterface> InventoryInterface(ActorComponent);
		if (!InventoryInterface)
		{
			continue;
		}

		if (UAOInventoryComponent* InventoryComponent = InventoryInterface->GetInventoryComponent())
		{
			OutInventoryComponents.Add(InventoryComponent);
		}
	}

	OutInventoryComponents.StableSort([](const UAOInventoryComponent& Left, const UAOInventoryComponent& Right)
	{
		return Left.GetUnifiedInventoryIntakePriority() > Right.GetUnifiedInventoryIntakePriority();
	});
}

void UAOInventoryStatics::AppendInventoryComponentsFromActor(const AActor* TargetActor, TArray<UAOInventoryComponent*>& OutInventoryComponents)
{
	TArray<UAOInventoryComponent*> CollectedInventoryComponents;
	CollectInventoryComponentsFromActor(TargetActor, CollectedInventoryComponents);

	for (UAOInventoryComponent* InventoryComponent : CollectedInventoryComponents)
	{
		if (InventoryComponent != nullptr)
		{
			OutInventoryComponents.Add(InventoryComponent);
		}
	}
}

bool UAOInventoryStatics::CanActorFullyAcceptInventoryBatch(const AActor* TargetActor, const FAOInventoryReceiveBatch& ReceiveBatch)
{
	if (ReceiveBatch.IsEmpty())
	{
		return true;
	}

	TArray<UAOInventoryComponent*> InventoryComponents;
	CollectInventoryComponentsFromActor(TargetActor, InventoryComponents);
	const UAOInventoryComponent* PreferredQuickBarComponent = FindPreferredQuickBarInventoryComponent(TargetActor, InventoryComponents);

	if (PreferredQuickBarComponent != nullptr && PreferredQuickBarComponent->CanFullyAcceptInventoryBatch(ReceiveBatch))
	{
		return true;
	}

	for (const UAOInventoryComponent* InventoryComponent : InventoryComponents)
	{
		if (InventoryComponent == nullptr || InventoryComponent == PreferredQuickBarComponent)
		{
			continue;
		}

		if (InventoryComponent->CanFullyAcceptInventoryBatch(ReceiveBatch))
		{
			return true;
		}
	}

	return false;
}

bool UAOInventoryStatics::TryAddInventoryBatchToActor(AActor* TargetActor, const FAOInventoryReceiveBatch& ReceiveBatch)
{
	if (ReceiveBatch.IsEmpty())
	{
		return true;
	}

	TArray<UAOInventoryComponent*> InventoryComponents;
	CollectInventoryComponentsFromActor(TargetActor, InventoryComponents);
	UAOInventoryComponent* PreferredQuickBarComponent = FindPreferredQuickBarInventoryComponent(TargetActor, InventoryComponents);

	if (PreferredQuickBarComponent != nullptr && PreferredQuickBarComponent->CanFullyAcceptInventoryBatch(ReceiveBatch))
	{
		return PreferredQuickBarComponent->TryAddInventoryBatch(ReceiveBatch);
	}

	for (UAOInventoryComponent* InventoryComponent : InventoryComponents)
	{
		if (InventoryComponent == nullptr || InventoryComponent == PreferredQuickBarComponent || !InventoryComponent->CanFullyAcceptInventoryBatch(ReceiveBatch))
		{
			continue;
		}

		return InventoryComponent->TryAddInventoryBatch(ReceiveBatch);
	}

	return false;
}
