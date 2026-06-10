// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Interaction/InteractableTarget.h"
#include "AegisOdyssey/Inventory/InventoryInterface.h"
#include "GameFramework/Actor.h"
#include "AOChest.generated.h"

class APawn;
class UAOContainerInventoryComponent;
class UAOInventoryComponent;
class UBoxComponent;
class UStaticMeshComponent;

// 通用世界容器对象的首版实现。
// 当前先以箱子为例，但职责仍然是“世界中的可交互容器对象”。
UCLASS(Blueprintable, BlueprintType)
class AEGISODYSSEY_API AAOChest : public AActor, public IInteractableTarget, public IInventoryInterface
{
	GENERATED_BODY()

public:
	AAOChest();

	virtual void GatherInteractionOptions(FInteractionOptionBuilder& OptionBuilder) override;
	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) override;
	virtual bool CanExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData) const override;
	virtual bool ExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData) override;
	virtual UAOInventoryComponent* GetInventoryComponent() override;

	bool CanInventoryAccessChest(const UAOInventoryComponent* InventoryComponent) const;
	bool IsInventoryOwnedByActiveSessionParticipant(const UAOInventoryComponent* InventoryComponent) const;

	bool TransferChestSlotToInventorySlot(UAOInventoryComponent* ExternalInventory, int32 ChestSlotIndex, int32 ExternalSlotIndex);
	bool TransferInventorySlotToChestSlot(UAOInventoryComponent* ExternalInventory, int32 ExternalSlotIndex, int32 ChestSlotIndex);
	bool TransferChestSlotToInventoryComponent(UAOInventoryComponent* TargetInventoryComponent, int32 ChestSlotIndex, int32 TargetSlotIndex);

	// 历史兼容壳：仍然保留“交互者默认接收入包”的蓝图入口。
	UFUNCTION(BlueprintCallable, Category = "AO|Chest")
	bool TransferItemToInteractorInventory(APawn* InteractingPawn, int32 ChestSlotIndex, int32 TargetSlotIndex);

	UFUNCTION(BlueprintPure, Category = "AO|Chest")
	UAOContainerInventoryComponent* GetChestInventoryComponent() const { return ChestInventory; }

protected:
	const FInteractionOption* FindInteractionOptionByIndex(int32 InteractionOptionIndex) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Chest")
	TObjectPtr<UBoxComponent> InteractionBounds = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Chest")
	TObjectPtr<UStaticMeshComponent> ChestMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Chest")
	TObjectPtr<UAOContainerInventoryComponent> ChestInventory = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Interaction")
	TArray<FInteractionOption> InteractionOptions;
};
