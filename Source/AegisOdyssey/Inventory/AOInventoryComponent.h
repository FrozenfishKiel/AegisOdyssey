// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryInterface.h"
#include "Components/GameFrameworkComponent.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySet.h"
#include "AegisOdyssey/Inventory/AOInventoryAcquisitionMessage.h"
#include "AegisOdyssey/Inventory/AOInventoryStatics.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "AOInventoryComponent.generated.h"

class UAOInventoryManagerComponent;
class UAOInventoryItemDefinition;
class UAOInventoryItemInstance;
class UAOInventoryComponent;
class UAOEquipmentInstance;
class UAbilitySystemComponent;
class APawn;
class UMVVM_InventoryMenu;
class UMVVM_InventoryItemContextMenu;
struct FAOInventoryList;
struct FAOItemCatalogRow;

USTRUCT(BlueprintType)
struct FAOInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FAOInventoryEntry() {}
	FAOInventoryEntry(UAOInventoryComponent* InInventoryComponentPtr)
		: SlotOwnerComponent(InInventoryComponentPtr)
	{
	}

	bool operator==(const FAOInventoryEntry& Other) const
	{
		return Instance == Other.Instance;
	}

	bool operator==(const UAOInventoryItemInstance* InInstance) const
	{
		if (SlotOwnerComponent == nullptr || InInstance == nullptr)
		{
			return false;
		}

		return Instance == InInstance;
	}

	bool operator!=(const UAOInventoryItemInstance* InInstance) const
	{
		return !(*this == InInstance);
	}

	void operator=(const FAOInventoryEntry& Other)
	{
		if (this != &Other)
		{
			Instance = Other.Instance;
			StackCount = Other.StackCount;
			GrantedHandles = Other.GrantedHandles;
			SlotOwnerComponent = Other.SlotOwnerComponent;
		}
	}

public:
	friend FAOInventoryList;
	friend UAOInventoryComponent;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAOInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 StackCount = 0;

	UPROPERTY()
	int32 LastObservedCount = 0;

	UPROPERTY(BlueprintReadOnly, NotReplicated)
	FAOAbilitySet_GrantedHandles GrantedHandles;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAOInventoryComponent> SlotOwnerComponent = nullptr;
};

USTRUCT(BlueprintType)
struct FAOInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FAOInventoryList() {}
	FAOInventoryList(UAOInventoryComponent* InInventoryItemDefinitionPtr)
		: OwnerComponent(InInventoryItemDefinitionPtr)
	{
	}
	~FAOInventoryList() {}

	TArray<UAOInventoryItemInstance*> GetAllItems() const;

private:
	friend UAOInventoryComponent;

public:
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FAOInventoryEntry, FAOInventoryList>(Entries, DeltaParms, *this);
	}

	UPROPERTY()
	TArray<FAOInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UAOInventoryComponent> OwnerComponent = nullptr;
};

template<>
struct TStructOpsTypeTraits<FAOInventoryList> : public TStructOpsTypeTraitsBase2<FAOInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};

UCLASS(Abstract)
class AEGISODYSSEY_API UAOInventoryComponent : public UGameFrameworkComponent, public IInventoryInterface
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnInventoryObservedChanged);

	UAOInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

	UAOInventoryItemInstance* AddEntry(TSubclassOf<UAOInventoryItemInstance> ItemClass,
		TSubclassOf<UAOInventoryItemDefinition> ItemDefClass, int32& ItemStackCount, bool& bCheck);
	void AddEntry(UAOInventoryItemInstance* Instance, int32& InCount, bool& bCheck);
	int32 FindAvaliableSlot(UAOInventoryItemInstance* Instance, int32 Count) const;

	virtual void InitializeOrRefreshInventorySlots();
	virtual void InitializeParams();
	void RemoveEntry(UAOInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category = Inventory, BlueprintPure = false)
	inline TArray<UAOInventoryItemInstance*> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category = Inventory, BlueprintPure = false)
	inline TArray<FAOInventoryEntry> GetAllLists() const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	virtual void AddItemInstance(UAOInventoryItemInstance* ItemInstance, int32& InCount, bool& bCheck);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	virtual void AddItemDefinition(TSubclassOf<UAOInventoryItemInstance> ItemClass,
		TSubclassOf<UAOInventoryItemDefinition> ItemDefClass, int32& StackCount, bool& bCheck);

	bool CanFullyAcceptItemDefinitions(const TArray<FAOItemCatalogRow>& ItemRows, const TArray<int32>& ItemCounts) const;
	bool TryAddItemDefinitionsBatch(const TArray<FAOItemCatalogRow>& ItemRows, const TArray<int32>& ItemCounts);

	bool CanFullyAcceptInventoryBatch(const FAOInventoryReceiveBatch& ReceiveBatch) const;
	bool TryAddInventoryBatch(const FAOInventoryReceiveBatch& ReceiveBatch);
	bool CanReceiveUnifiedInventoryIntake() const { return bAllowUnifiedInventoryIntake; }
	int32 GetUnifiedInventoryIntakePriority() const { return UnifiedInventoryIntakePriority; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	void RemoveItemInstance(UAOInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	void RemoveItemInstanceFromIndex(int32 TargetIndex);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	bool ConsumeItemAtSlot(int32 SlotIndex, int32 ConsumeCount);

	UFUNCTION(BlueprintPure, Category = Inventory)
	bool IsValidInventorySlotIndex(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = Inventory)
	bool HasItemAtSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = Inventory)
	int32 FindInventorySlotIndexFromInstance(const UAOInventoryItemInstance* ItemInstance) const;

	int32 GetInventorySlotCount() const;
	const FAOInventoryEntry* GetInventoryEntryAtSlot(int32 SlotIndex) const;
	bool CanUseItemAtSlot(int32 SlotIndex, APawn* UserPawn = nullptr) const;

	// 取出或创建当前库存组件持有的右键菜单主 ViewModel。
	// InventoryComponent 在这条链路里只负责“存一份对象”，不再负责推断宿主或来源。
	// 外部谁要消费这份 ViewModel，就自己先拿到目标 InventoryComponent，再从它这里取。
	UMVVM_InventoryItemContextMenu* GetOrCreateContextMenuViewModel();

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool TryUseItemAtSlot(int32 SlotIndex, APawn* UserPawn = nullptr);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void WhenItemExchange(UAOInventoryComponent* DropInventory, int32 DraggedSlotIndex, int32 DropSlotIndex);

	static bool CanExecuteExchangeRequest(UAOInventoryComponent* DraggedInventory, int32 DraggedSlotIndex,
		UAOInventoryComponent* DropInventory, int32 DropSlotIndex);
	static bool ExecuteExchangeRequestOnAuthority(UAOInventoryComponent* DraggedInventory, int32 DraggedSlotIndex,
		UAOInventoryComponent* DropInventory, int32 DropSlotIndex);
	static bool ExecuteExchangeRequest(UAOInventoryComponent* DraggedInventory, int32 DraggedSlotIndex,
		UAOInventoryComponent* DropInventory, int32 DropSlotIndex);

	FAOInventoryEntry FindInventoryEntryFromInstance(UAOInventoryItemInstance* ItemInstance) const;
	virtual UAOInventoryComponent* GetInventoryComponent() override { return this; }
	virtual inline TArray<FAOInventoryEntry> GetInventoryContainer() const { return InventoryList.Entries; }

public:
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void BroadCastInventoryRemoveOnClient(const TArrayView<int32> RemovedIndices, int32 FinalSize) {}
	virtual void BroadCastInventoryAddOnClient(const TArrayView<int32> AddIndices, int32 FinalSize, const TArray<FAOInventoryEntry>& TargetList) {}
	virtual void BroadCastInventoryChangeOnClient(const TArrayView<int32> ChangedIndices, int32 FinalSize) {}
	virtual void BroadCastInventoryChange(int32 ChangedIndex = 0);
	virtual bool CanAcceptInventoryEntryAtSlot(const FAOInventoryEntry& IncomingEntry, int32 TargetSlotIndex) const { return IsValidInventorySlotIndex(TargetSlotIndex); }

	FOnInventoryObservedChanged OnInventoryObservedChanged;

protected:
	struct FSimulatedInventorySlot
	{
		const UAOInventoryItemDefinition* ItemDefinition = nullptr;
		int32 StackCount = 0;
	};

	// 按 Definition 解析最终应生成的 Instance 类，并创建真正要入库的物品实例。
	UAOInventoryItemInstance* CreateInventoryItemInstance(TSubclassOf<UAOInventoryItemInstance> ItemClass,
		TSubclassOf<UAOInventoryItemDefinition> ItemDefClass);

	// 在“模拟库存槽位”里尝试放入一批同类物品。
	// 这里只做容量预演，不会修改正式库存。
	bool SimulateAddItemDefinition(FSimulatedInventorySlot& SimulatedSlot, const UAOInventoryItemDefinition& ItemDefinition,
		int32& InOutRemainingCount) const;
	// 用整批输入预演一次“如果现在入库，最终能不能全部放下”。
	// 成功时，SimulatedSlots 会变成预演后的库存快照。
	bool SimulateAddItemDefinitionBatch(const TArray<FAOItemCatalogRow>& ItemRows, const TArray<int32>& ItemCounts,
		TArray<FSimulatedInventorySlot>& SimulatedSlots) const;
	int32 GetMaxStackCountForDefinition(const UAOInventoryItemDefinition& ItemDefinition) const;

	// 某个槽位被清空后，要确认这个实例有没有被别的槽位继续共用。
	// 只有完全没有其他引用时，才把它从复制子对象列表里移除。
	bool IsItemInstanceReferencedByOtherSlots(const UAOInventoryItemInstance* ItemInstance, int32 IgnoredSlotIndex) const;
	void UpdateReplicatedItemRegistration(UAOInventoryItemInstance* ItemInstance, UAOInventoryComponent* PreviousOwnerComponent,
		UAOInventoryComponent* NewOwnerComponent);

	void DispatchInventoryAcquisitionMessages(const TArray<FAOInventoryAcquisitionMessage>& Messages);
	void BuildInventoryAcquisitionMessagesFromReceiveBatch(const FAOInventoryReceiveBatch& ReceiveBatch, TArray<FAOInventoryAcquisitionMessage>& OutMessages) const;
	void BuildInventoryAcquisitionMessagesFromItemRows(const TArray<FAOItemCatalogRow>& ItemRows, const TArray<int32>& ItemCounts, TArray<FAOInventoryAcquisitionMessage>& OutMessages) const;
	void BroadcastInventoryAcquisitionMessagesLocally(const TArray<FAOInventoryAcquisitionMessage>& Messages) const;

	UFUNCTION(Client, Reliable)
	void ClientBroadcastInventoryAcquisitionMessages(const TArray<FAOInventoryAcquisitionMessage>& Messages);

	UFUNCTION(Server, Reliable)
	void ServerTryUseItemAtSlot(int32 SlotIndex, APawn* UserPawn);

	UPROPERTY(Replicated)
	FAOInventoryList InventoryList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InventoryConfig")
	int32 NumSlots = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InventoryConfig")
	bool bAllowUnifiedInventoryIntake = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InventoryConfig")
	int32 UnifiedInventoryIntakePriority = 0;

	// 当前 InventoryComponent 持有的右键菜单主 ViewModel。
	// 这份对象的作用域和生命周期直接跟着组件走，不再经过额外的宿主解析逻辑。
	UPROPERTY(Transient)
	TObjectPtr<UMVVM_InventoryItemContextMenu> ContextMenuViewModel = nullptr;
};
