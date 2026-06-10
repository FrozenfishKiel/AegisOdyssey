#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentTypes.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "Input/Reply.h"
#include "AOFormalEquipmentSlotUI.generated.h"

class UAOFormalEquipmentManagerComponent;
class UAOFormalEquipmentSlotInventoryComponent;
class UAOInventoryComponent;
class UAOInventoryItemInstance;
class UDragDropOperation;
class UImage;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOFormalEquipmentSlotUI : public UAOInventoryUI
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UFUNCTION(BlueprintCallable, Category = "AO|FormalEquipment UI")
	void SetObservedSlotData(int32 InObservedSlotIndex, EAOFormalEquipmentSlotType InFormalSlotType, UAOInventoryComponent* InSourceContainer);

	UFUNCTION(BlueprintCallable, Category = "AO|FormalEquipment UI")
	void SetObservedSlotEntry(const FAOInventoryEntry& InObservedSlotEntry);

	UFUNCTION(BlueprintCallable, Category = "AO|FormalEquipment UI")
	void RefreshFormalEquipmentSlotDisplay();

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment UI")
	UAOFormalEquipmentManagerComponent* GetOwningFormalEquipmentManager() const;

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment UI")
	UAOFormalEquipmentSlotInventoryComponent* GetOwningFormalEquipmentSlotInventory() const;

	UFUNCTION(BlueprintCallable, Category = "AO|FormalEquipment UI")
	void SetOwningFormalEquipmentManager(UAOFormalEquipmentManagerComponent* InFormalEquipmentManager);

	UFUNCTION(BlueprintCallable, Category = "AO|FormalEquipment UI")
	void SetOwningFormalEquipmentSlotInventory(UAOFormalEquipmentSlotInventoryComponent* InFormalEquipmentSlotInventory);

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment UI")
	int32 GetObservedSlotIndex() const { return ObservedSlotIndex; }

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment UI")
	int32 GetIndex() const { return Index; }

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment UI")
	EAOFormalEquipmentSlotType GetFormalSlotType() const { return FormalSlotType; }

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment UI")
	FText GetFormalSlotDisplayName() const { return FormalSlotDisplayName; }

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment UI|DragDrop")
	UAOInventoryComponent* GetSourceContainer() const { return SourceContainer; }

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment UI|DragDrop")
	bool CanAcceptDraggedSourceSlotForThisFormalSlot(UAOInventoryComponent* InSourceContainer, int32 InSourceSlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "AO|FormalEquipment UI|DragDrop")
	bool CanAcceptDraggedItemForThisFormalSlot(const UAOInventoryItemInstance* SourceItemInstance) const;

	UFUNCTION(BlueprintCallable, Category = "AO|FormalEquipment UI|DragDrop")
	bool RequestEquipDraggedSourceSlotToThisFormalSlot(UAOInventoryComponent* InSourceContainer, int32 InSourceSlotIndex);

protected:
	virtual TArray<FAOInventoryItemContextAction> BuildInventoryItemContextActions(
		UAOInventoryComponent* SourceInventory,
		int32 SourceSlotIndex,
		UAOInventoryItemInstance* ItemInstance) const;

	virtual bool ExecuteInventoryItemContextAction(
		const FAOInventoryItemContextAction& Action,
		UAOInventoryComponent* SourceInventory,
		int32 SourceSlotIndex,
		UAOInventoryItemInstance* ItemInstance,
		FName CraftingRecipeRowName = NAME_None);

	virtual bool ResolveInventoryItemContextMenuRequest(
		UAOInventoryComponent*& OutSourceInventory,
		int32& OutSourceSlotIndex,
		UAOInventoryItemInstance*& OutItemInstance) const override;

	virtual const UAOInventoryItemDefinition* ResolveHoverTooltipItemDefinition() const override;
	virtual void HandleFormalEquipmentSlotUpdated(bool bHasValidItem);
	bool ResolveCurrentSlotEntry(FAOInventoryEntry& OutEntry) const;
	bool ResolveDraggedSourceSlotFromOperation(
		const UDragDropOperation* InOperation,
		UAOInventoryComponent*& OutSourceInventory,
		int32& OutSourceSlotIndex,
		UAOInventoryItemInstance*& OutItemInstance) const;
	FText BuildFormalSlotLabel() const;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> ItemIcon = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SlotNameText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemNameText = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AO|FormalEquipment UI")
	int32 Index = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "AO|FormalEquipment UI")
	int32 ObservedSlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "AO|FormalEquipment UI")
	EAOFormalEquipmentSlotType FormalSlotType = EAOFormalEquipmentSlotType::None;

	UPROPERTY(BlueprintReadOnly, Category = "AO|FormalEquipment UI")
	FText FormalSlotDisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "AO|FormalEquipment UI|DragDrop")
	TObjectPtr<UAOInventoryComponent> SourceContainer = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AO|FormalEquipment UI")
	FAOInventoryEntry ObservedSlotEntry;

	UPROPERTY(BlueprintReadOnly, Category = "AO|FormalEquipment UI")
	TObjectPtr<UAOFormalEquipmentManagerComponent> FormalEquipmentManager = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AO|FormalEquipment UI")
	TObjectPtr<UAOFormalEquipmentSlotInventoryComponent> FormalEquipmentSlotInventory = nullptr;
};
