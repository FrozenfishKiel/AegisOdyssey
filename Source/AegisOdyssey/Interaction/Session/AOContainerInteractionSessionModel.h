// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Interaction/Session/AOInteractionSessionModel.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryUI.h"
#include "AOContainerInteractionSessionModel.generated.h"

class UAOInventoryComponent;
class UAOInventoryItemDefinition;
class UAOInventoryItemInstance;
class UAOBackPackComponent;
class UAOQuickBarComponent;
class UAOFormalEquipmentManagerComponent;
class UAOFormalEquipmentSlotInventoryComponent;
class UAOSkillComponent;
class UAOSkillSlotInventoryComponent;
class UMVVM_InventoryMenu;
class APawn;

UENUM()
enum class EAOContainerSessionMutationType : uint8
{
	None,
	ExchangeInventorySlots,
	UseInventoryItem
};

USTRUCT()
struct FAOContainerSessionMutationRequest
{
	GENERATED_BODY()

	UPROPERTY()
	EAOContainerSessionMutationType MutationType = EAOContainerSessionMutationType::None;

	UPROPERTY()
	TObjectPtr<UAOInventoryComponent> SourceInventory = nullptr;

	UPROPERTY()
	int32 SourceSlotIndex = INDEX_NONE;

	UPROPERTY()
	TObjectPtr<UAOInventoryComponent> TargetInventory = nullptr;

	UPROPERTY()
	int32 TargetSlotIndex = INDEX_NONE;

	UPROPERTY()
	TObjectPtr<APawn> UserPawn = nullptr;
};

USTRUCT(BlueprintType)
struct FAOObservedInventorySlot
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAOInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UAOInventoryItemDefinition> ItemDefClass = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 StackCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = INDEX_NONE;
};

UCLASS(BlueprintType)
class AEGISODYSSEY_API UAOContainerInteractionSessionModel : public UAOInteractionSessionModel
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnContainerDataChanged);

	void InitializeContainerSession(AActor* InInteractableActor, UAOInventoryComponent* InInventoryComponent);
	void ApplyObservedSlotsSnapshot(const TArray<FAOObservedInventorySlot>& InObservedSlots);

	virtual void ActivateSession(UAOInteractionSessionComponent* InOwnerSessionComponent) override;
	virtual void DeactivateSession() override;

	void RefreshObservedContainer();

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	bool HasObservedContainer() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOInventoryComponent* GetCurrentContainerInventory() const { return ObservedInventoryComponent.Get(); }

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	bool IsObservedTargetInventoryComponent(const UAOInventoryComponent* InventoryComponent) const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	bool UsesObservedTargetInventory(const UAOInventoryComponent* FirstInventoryComponent, const UAOInventoryComponent* SecondInventoryComponent) const;

	bool CanExecuteMutationRequest(const FAOContainerSessionMutationRequest& MutationRequest) const;
	bool ExecuteMutationRequestOnAuthority(const FAOContainerSessionMutationRequest& MutationRequest);

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	const TArray<FAOObservedInventorySlot>& GetObservedContainerSlots() const { return ObservedContainerSlots; }

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UMVVM_InventoryMenu* GetContainerViewModel() const { return ContainerViewModel; }

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOBackPackComponent* GetObservedBackPackComponent() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOQuickBarComponent* GetObservedQuickBarComponent() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UMVVM_InventoryMenu* GetTargetQuickBarViewModel() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOFormalEquipmentSlotInventoryComponent* GetObservedFormalEquipmentSlotInventory() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UMVVM_InventoryMenu* GetTargetFormalEquipmentViewModel() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOFormalEquipmentManagerComponent* GetObservedFormalEquipmentManager() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOSkillComponent* GetObservedSkillComponent() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOSkillSlotInventoryComponent* GetObservedSkillSlotInventory() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	TArray<FAOSkillSlotViewData> GetObservedSkillSlotViewDataList() const;

	void PopulateTargetInventoryDisplayContext(FAOInventoryDisplayContext& OutDisplayContext) const;

	FOnContainerDataChanged& GetOnContainerDataChanged() { return OnContainerDataChanged; }

protected:
	void RegisterObservedTargetComponents(AActor* InInteractableActor);
	void BindToObservedInventoryChanges();
	void UnbindFromObservedInventoryChanges();
	void BroadcastObservedContainerChanged();
	void EnsureViewModel();
	void HandleObservedInventoryChanged();
	void SetObservedSlotsFromInventory(const UAOInventoryComponent* InInventoryComponent);
	void ResolveInventoryComponentFromInteractableActor();
	bool CanExecuteExchangeMutationRequest(const FAOContainerSessionMutationRequest& MutationRequest) const;
	bool CanExecuteUseMutationRequest(const FAOContainerSessionMutationRequest& MutationRequest) const;

protected:
	UPROPERTY(Transient)
	TWeakObjectPtr<UAOInventoryComponent> ObservedInventoryComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAOBackPackComponent> ObservedBackPackComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAOQuickBarComponent> ObservedQuickBarComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAOFormalEquipmentSlotInventoryComponent> ObservedFormalEquipmentSlotInventory;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAOFormalEquipmentManagerComponent> ObservedFormalEquipmentManager;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAOSkillComponent> ObservedSkillComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAOSkillSlotInventoryComponent> ObservedSkillSlotInventory;

	UPROPERTY(Transient)
	TArray<FAOObservedInventorySlot> ObservedContainerSlots;

	UPROPERTY(Transient)
	TObjectPtr<UMVVM_InventoryMenu> ContainerViewModel = nullptr;

	FOnContainerDataChanged OnContainerDataChanged;
	FDelegateHandle ObservedInventoryChangedHandle;
};
