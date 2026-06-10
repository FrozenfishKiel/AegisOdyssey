#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/AOInventoryItemContextMenuTypes.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryMenu.h"
#include "CommonUserWidget.h"
#include "AOInventoryUI.generated.h"

class AActor;
class APlayerController;
class UAOContainerInteractionSessionModel;
class UAOInteractionSessionComponent;
class UAOInventoryComponent;
class UAOInventoryItemContextMenuWidget;
class UAOInventoryItemDefinition;
class UAOInventoryItemInstance;
class UAOBackPackComponent;
class UAOFormalEquipmentManagerComponent;
class UAOFormalEquipmentSlotInventoryComponent;
class UAOQuickBarComponent;
class UAOSkillComponent;
class UAOSkillSlotInventoryComponent;
class UDragDropOperation;
class UMVVM_Crafting;
class UMVVM_ItemHoverTooltip;

USTRUCT(BlueprintType)
struct FAOInventorySlot
{
	GENERATED_BODY()

	FAOInventorySlot() = default;

	void operator=(const FAOInventoryEntry& Other)
	{
		Instance = Other.Instance;
		StackCount = Other.StackCount;
		LastObservedCount = Other.LastObservedCount;
		SlotOwnerComponent = Other.SlotOwnerComponent;
	}

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAOInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 StackCount = 0;

	UPROPERTY()
	int32 LastObservedCount = 0;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAOInventoryComponent> SlotOwnerComponent = nullptr;
};

USTRUCT(BlueprintType)
struct FAOInventoryDisplayContext
{
	GENERATED_BODY()

	// This is a page-level display snapshot.
	// It only tells child panels which actor/components they should read from right now.
	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory UI")
	TObjectPtr<AActor> OwnerActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory UI")
	TObjectPtr<UAOBackPackComponent> BackPackComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory UI")
	TObjectPtr<UAOQuickBarComponent> QuickBarComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory UI")
	TObjectPtr<UAOFormalEquipmentSlotInventoryComponent> FormalEquipmentInventory = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory UI")
	TObjectPtr<UAOFormalEquipmentManagerComponent> FormalEquipmentManager = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory UI")
	TObjectPtr<UAOSkillComponent> SkillComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AO|Inventory UI")
	TObjectPtr<UAOSkillSlotInventoryComponent> SkillSlotInventory = nullptr;
};

UCLASS()
class AEGISODYSSEY_API UAOInventoryUI : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeDestruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UFUNCTION(BlueprintPure, BlueprintCallable)
	virtual UMVVM_InventoryMenu* GetInventoryViewModel() const;

	UFUNCTION(BlueprintPure, Category = "AO|UI")
	APlayerController* GetOwningAOPlayerController() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOInteractionSessionComponent* GetOwningInteractionSessionComponent() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOContainerInteractionSessionModel* GetOwningContainerSessionModel() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void RequestAcquireCurrentInteractableOwner();

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void RequestReleaseCurrentInteractableOwner();

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void RequestExchangeBetweenInventories(
		UAOInventoryComponent* DraggedInventory,
		int32 DraggedSlotIndex,
		UAOInventoryComponent* DropInventory,
		int32 DropSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void RequestUseInventoryItem(UAOInventoryComponent* SourceInventory, int32 SourceSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void RequestOpenInventoryItemContextMenu(
		UAOInventoryComponent* SourceInventory,
		int32 SourceSlotIndex,
		UAOInventoryItemInstance* ItemInstance,
		const FVector2D& ScreenSpacePosition);

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void RequestOpenCraftingRecipeContextMenu(
		FName RecipeRowName,
		UAOInventoryItemDefinition* ItemDefinition,
		const FText& InfoText,
		const FVector2D& ScreenSpacePosition);

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	bool TryResolveInventorySlotContext(
		UAOInventoryComponent*& OutSourceInventory,
		int32& OutSourceSlotIndex,
		UAOInventoryItemInstance*& OutItemInstance) const;

	UFUNCTION(BlueprintPure, Category = "AO|Inventory UI")
	UMVVM_ItemHoverTooltip* GetItemHoverTooltipViewModel() const;

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

	bool ShouldRouteInventoryExchangeThroughInteractionSession(
		UAOInventoryComponent* DraggedInventory,
		UAOInventoryComponent* DropInventory) const;

	bool ShouldRouteInventoryUseThroughInteractionSession(UAOInventoryComponent* SourceInventory) const;

	void HandleInventoryItemContextMenuClosed(UAOInventoryItemContextMenuWidget* ClosedMenuWidget);

protected:
	virtual bool ResolveInventoryItemContextMenuRequest(
		UAOInventoryComponent*& OutSourceInventory,
		int32& OutSourceSlotIndex,
		UAOInventoryItemInstance*& OutItemInstance) const;
	virtual bool ResolveDropTargetInventorySlot(
		UAOInventoryComponent*& OutTargetInventory,
		int32& OutTargetSlotIndex) const;
	bool ResolveDraggedInventorySlotFromDropOperation(
		const UDragDropOperation* InOperation,
		UAOInventoryComponent*& OutSourceInventory,
		int32& OutSourceSlotIndex,
		UAOInventoryItemInstance*& OutItemInstance) const;

	virtual const UAOInventoryItemDefinition* ResolveHoverTooltipItemDefinition() const;
	UAOBackPackComponent* ResolveBackPackComponentFromDisplayContext(const FAOInventoryDisplayContext& InDisplayContext) const;
	UAOQuickBarComponent* ResolveQuickBarComponentFromDisplayContext(const FAOInventoryDisplayContext& InDisplayContext) const;
	UAOFormalEquipmentSlotInventoryComponent* ResolveFormalEquipmentInventoryFromDisplayContext(
		const FAOInventoryDisplayContext& InDisplayContext) const;
	UAOFormalEquipmentManagerComponent* ResolveFormalEquipmentManagerFromDisplayContext(
		const FAOInventoryDisplayContext& InDisplayContext) const;
	UAOSkillComponent* ResolveSkillComponentFromDisplayContext(const FAOInventoryDisplayContext& InDisplayContext) const;
	UAOSkillSlotInventoryComponent* ResolveSkillSlotInventoryFromDisplayContext(
		const FAOInventoryDisplayContext& InDisplayContext) const;

	UPROPERTY(EditDefaultsOnly, Category = "AO|Interaction")
	TSubclassOf<UAOInventoryItemContextMenuWidget> InventoryItemContextMenuClass;

	UPROPERTY(Transient)
	TObjectPtr<UAOInventoryItemContextMenuWidget> ActiveContextMenuWidget = nullptr;

private:
	void ShowHoverTooltip(const FGeometry& InGeometry);
	void HideHoverTooltip();
};
