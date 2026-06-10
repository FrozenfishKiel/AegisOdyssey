// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Interaction/InteractableTarget.h"
#include "AegisOdyssey/Interaction/InteractionOption.h"
#include "AegisOdyssey/Interaction/PickUpable.h"
#include "AOInventoryItemInstance.generated.h"

class UAOInventoryItemDefinition;
class UAOInventoryManagerComponent;
class AActor;
class APawn;
struct FAOInventoryEntry;

UCLASS(BlueprintType, Blueprintable)
class AEGISODYSSEY_API UAOInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UAOInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void SetItemDef(TSubclassOf<UAOInventoryItemDefinition> InDef);
	UAOInventoryItemDefinition* GetItemCDO() const;
	void SetRuntimeOwnerActor(AActor* InRuntimeOwnerActor);
	AActor* GetRuntimeOwnerActor() const;

	friend struct FAOInventoryList;

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	TSubclassOf<UAOInventoryItemDefinition> ItemDef;

	UPROPERTY(BlueprintReadOnly, Replicated)
	TObjectPtr<UAOInventoryItemDefinition> ItemCDO;

	UPROPERTY(BlueprintReadOnly, Replicated)
	TObjectPtr<AActor> RuntimeOwnerActor = nullptr;

	virtual UAOInventoryManagerComponent* FindTargetInventoryManager() const;
	virtual bool CanUseFromInventory(const FAOInventoryEntry& InventoryEntry, APawn* UserPawn) const;
	virtual bool TryUseFromInventory(FAOInventoryEntry& InventoryEntry, APawn* UserPawn, int32& OutConsumeCount);
	virtual bool IsSupportedForNetworking() const override { return true; }

protected:
	bool CanUseConsumableFromInventory() const;
	bool TryApplyConsumableUseEffects(APawn* UserPawn) const;
};

