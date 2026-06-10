// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "AOInventorySlotBase.generated.h"

class UAOInventoryComponent;
class UAOInventoryItemInstance;

UCLASS(Abstract)
class AEGISODYSSEY_API UAOInventorySlotBase : public UAOInventoryUI
{
	GENERATED_BODY()

public:
	void SetSlotIndex(int32 InIndex);
	void SetSourceContainer(UAOInventoryComponent* InSourceContainer);
	void SetItemInstance(UAOInventoryItemInstance* InItemInstance);
	void SetSlotContext(int32 InIndex, UAOInventoryComponent* InSourceContainer, UAOInventoryItemInstance* InItemInstance = nullptr);

protected:
	virtual bool ResolveInventoryItemContextMenuRequest(
		UAOInventoryComponent*& OutSourceInventory,
		int32& OutSourceSlotIndex,
		UAOInventoryItemInstance*& OutItemInstance) const override;

	virtual bool ResolveDropTargetInventorySlot(
		UAOInventoryComponent*& OutTargetInventory,
		int32& OutTargetSlotIndex) const override;

protected:
	UPROPERTY(BlueprintReadWrite, Category = "AO|Inventory Slot")
	TObjectPtr<UAOInventoryItemInstance> ItemInstance = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Inventory Slot")
	int32 Index = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, Category = "AO|Inventory Slot")
	TObjectPtr<UAOInventoryComponent> SourceContainer = nullptr;
};
