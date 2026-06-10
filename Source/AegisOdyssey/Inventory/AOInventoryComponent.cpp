// Fill out your copyright notice in the Description page of Project Settings.


#include "AOInventoryComponent.h"

#include "AOInventoryItemDefinition.h"
#include "AOInventoryIteminstance.h"
#include "AOInventoryMessageSubsystem.h"
#include "AegisOdyssey/Items/AOItemCatalogTypes.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.h"
#include "Engine/ActorChannel.h"
#include "Fragments/AOFragment_SetStats.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryComponent)

namespace AOInventoryComponentPrivate
{
	bool ShouldForwardAcquisitionMessagesToOwningClient(const AActor* OwningActor)
	{
		if (OwningActor == nullptr || OwningActor->GetNetMode() == NM_Standalone)
		{
			return false;
		}

		if (const APawn* OwningPawn = Cast<APawn>(OwningActor))
		{
			if (OwningPawn->IsLocallyControlled())
			{
				return false;
			}
		}

		return true;
	}
}

UAOInventoryComponent::UAOInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UAOInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UAOInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAOInventoryComponent::BroadCastInventoryChange(int32 ChangedIndex)
{
	(void)ChangedIndex;
	OnInventoryObservedChanged.Broadcast();
}

UMVVM_InventoryItemContextMenu* UAOInventoryComponent::GetOrCreateContextMenuViewModel()
{
	if (ContextMenuViewModel == nullptr)
	{
		// ViewModel 的 Outer 直接挂在当前库存组件上，
		// 这样生命周期、GC 和调试归属都跟着这一个明确来源走，不再额外做宿主解析或来源兜底。
		ContextMenuViewModel = NewObject<UMVVM_InventoryItemContextMenu>(this);
	}

	return ContextMenuViewModel;
}

UAOInventoryItemInstance* UAOInventoryComponent::CreateInventoryItemInstance(
	TSubclassOf<UAOInventoryItemInstance> ItemClass,
	TSubclassOf<UAOInventoryItemDefinition> ItemDefClass)
{
	if (ItemDefClass == nullptr)
	{
		return nullptr;
	}

	AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr)
	{
		return nullptr;
	}

	const TSubclassOf<UAOInventoryItemInstance> ResolvedItemClass =
		UAOInventoryItemDefinition::ResolveItemInstanceClass(ItemDefClass, ItemClass);
	if (ResolvedItemClass == nullptr)
	{
		return nullptr;
	}

	UAOInventoryItemInstance* NewInstance = NewObject<UAOInventoryItemInstance>(OwningActor, ResolvedItemClass);
	if (NewInstance == nullptr)
	{
		return nullptr;
	}

	NewInstance->SetItemDef(ItemDefClass);
	NewInstance->SetRuntimeOwnerActor(OwningActor);
	return NewInstance;
}

void UAOInventoryComponent::UpdateReplicatedItemRegistration(
	UAOInventoryItemInstance* ItemInstance,
	UAOInventoryComponent* PreviousOwnerComponent,
	UAOInventoryComponent* NewOwnerComponent)
{
	if (ItemInstance == nullptr || PreviousOwnerComponent == NewOwnerComponent)
	{
		return;
	}

	if (PreviousOwnerComponent != nullptr && PreviousOwnerComponent->IsUsingRegisteredSubObjectList())
	{
		PreviousOwnerComponent->RemoveReplicatedSubObject(ItemInstance);
	}

	if (NewOwnerComponent != nullptr && NewOwnerComponent->IsUsingRegisteredSubObjectList() && NewOwnerComponent->IsReadyForReplication())
	{
		NewOwnerComponent->AddReplicatedSubObject(ItemInstance);
	}
}

namespace
{
	static void SyncInventoryItemRuntimeOwner(UAOInventoryItemInstance* ItemInstance, const UAOInventoryComponent* OwningInventoryComponent)
	{
		if (ItemInstance == nullptr)
		{
			return;
		}

		ItemInstance->SetRuntimeOwnerActor(
			OwningInventoryComponent != nullptr ? OwningInventoryComponent->GetOwner() : nullptr);
	}
}

void UAOInventoryComponent::WhenItemExchange_Implementation(
	UAOInventoryComponent* DropInventory,
	const int32 DraggedSlotIndex,
	const int32 DropSlotIndex)
{
	if (DropInventory == nullptr)
	{
		return;
	}

	FAOInventoryEntry& DropEntry = DropInventory->InventoryList.Entries[DropSlotIndex];
	FAOInventoryEntry& DraggedEntry = InventoryList.Entries[DraggedSlotIndex];
	UAOInventoryItemInstance* DraggedInstanceBefore = DraggedEntry.Instance;
	UAOInventoryItemInstance* DropInstanceBefore = DropEntry.Instance;

	auto ClearEntry = [](FAOInventoryEntry& Entry)
	{
		Entry = FAOInventoryEntry();
	};

	auto SwapEntries = [](FAOInventoryEntry& LeftEntry, FAOInventoryEntry& RightEntry)
	{
		const FAOInventoryEntry TempEntry = LeftEntry;
		LeftEntry = RightEntry;
		RightEntry = TempEntry;
	};

	if (DraggedEntry.Instance == nullptr && DropEntry.Instance == nullptr)
	{
		return;
	}

	if (DraggedEntry.Instance == nullptr || DropEntry.Instance == nullptr)
	{
		SwapEntries(DraggedEntry, DropEntry);
	}
	else
	{
		const UAOInventoryItemDefinition* DraggedDefinition = DraggedEntry.Instance->GetItemCDO();
		const UAOInventoryItemDefinition* DropDefinition = DropEntry.Instance->GetItemCDO();
		if (DraggedDefinition == nullptr || DropDefinition == nullptr)
		{
			return;
		}

		const UAOFragment_SetStats* DraggedStats = DraggedDefinition->FindFragmentByClass<UAOFragment_SetStats>();
		const UAOFragment_SetStats* DropStats = DropDefinition->FindFragmentByClass<UAOFragment_SetStats>();
		const bool bCanStackTogether =
			DraggedStats != nullptr &&
			DropStats != nullptr &&
			DraggedStats->CanStack &&
			DropStats->CanStack &&
			DraggedDefinition->DisplayName == DropDefinition->DisplayName;

		if (!bCanStackTogether)
		{
			SwapEntries(DraggedEntry, DropEntry);
		}
		else
		{
			const int32 MaxStackCount = FMath::Max(1, DraggedStats->MaxStack);
			if (DraggedEntry.StackCount >= MaxStackCount)
			{
				return;
			}

			int32 RemainingDropCount = DropEntry.StackCount - (MaxStackCount - DraggedEntry.StackCount);
			if (RemainingDropCount <= 0)
			{
				DraggedEntry.StackCount += DropEntry.StackCount;
				ClearEntry(DropEntry);
			}
			else
			{
				DraggedEntry.StackCount = FMath::Clamp(DraggedEntry.StackCount + DropEntry.StackCount, DraggedEntry.StackCount, MaxStackCount);
				DropEntry.StackCount = RemainingDropCount;
			}
		}
	}

	auto ResolveCurrentOwnerForInstance = [&](UAOInventoryItemInstance* ItemInstance) -> UAOInventoryComponent*
	{
		if (ItemInstance == nullptr)
		{
			return nullptr;
		}

		if (DraggedEntry.Instance == ItemInstance)
		{
			return this;
		}

		if (DropEntry.Instance == ItemInstance)
		{
			return DropInventory;
		}

		return nullptr;
	};

	UpdateReplicatedItemRegistration(DraggedInstanceBefore, this, ResolveCurrentOwnerForInstance(DraggedInstanceBefore));
	if (DropInstanceBefore != DraggedInstanceBefore)
	{
		UpdateReplicatedItemRegistration(DropInstanceBefore, DropInventory, ResolveCurrentOwnerForInstance(DropInstanceBefore));
	}

	DraggedEntry.SlotOwnerComponent = this;
	DropEntry.SlotOwnerComponent = DropInventory;
	SyncInventoryItemRuntimeOwner(DraggedEntry.Instance, this);
	SyncInventoryItemRuntimeOwner(DropEntry.Instance, DropInventory);

	if ((GetOwner() != nullptr && GetOwner()->HasAuthority()) || (DropInventory->GetOwner() != nullptr && DropInventory->GetOwner()->HasAuthority()))
	{
		BroadCastInventoryChange(DraggedSlotIndex);
		DropInventory->BroadCastInventoryChange(DropSlotIndex);
	}

	InventoryList.MarkItemDirty(DraggedEntry);
	DropInventory->InventoryList.MarkItemDirty(DropEntry);
}

UAOInventoryItemInstance* UAOInventoryComponent::AddEntry(
	TSubclassOf<UAOInventoryItemInstance> ItemClass,
	TSubclassOf<UAOInventoryItemDefinition> ItemDefClass,
	int32& ItemStackCount,
	bool& bCheck)
{
	if (ItemDefClass == nullptr)
	{
		bCheck = false;
		return nullptr;
	}

	if (InventoryList.Entries.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("UAOInventoryComponent::AddEntry: InventoryList.Entries is empty."));
		bCheck = false;
		return nullptr;
	}

	AActor* OwningActor = GetOwner();
	check(OwningActor->HasAuthority());

	UAOInventoryItemInstance* TemplateInstance = CreateInventoryItemInstance(ItemClass, ItemDefClass);
	if (TemplateInstance == nullptr)
	{
		bCheck = false;
		return nullptr;
	}

	UAOInventoryItemDefinition* TargetItemCDO = TemplateInstance->GetItemCDO();
	if (TargetItemCDO == nullptr)
	{
		bCheck = false;
		return nullptr;
	}

	const UAOFragment_SetStats* StatsFrag = TargetItemCDO->FindFragmentByClass<UAOFragment_SetStats>();
	bool bAddedAnything = false;
	UAOInventoryItemInstance* FirstCreatedInstance = nullptr;
	bool bTemplateRegistered = false;

	for (int32 SlotIndex = 0; SlotIndex < InventoryList.Entries.Num() && ItemStackCount > 0; ++SlotIndex)
	{
		FAOInventoryEntry& Entry = InventoryList.Entries[SlotIndex];
		if (Entry.Instance == nullptr)
		{
			continue;
		}

		UAOInventoryItemDefinition* SourceItemCDO = Entry.Instance->GetItemCDO();
		if (SourceItemCDO == nullptr || TargetItemCDO->DisplayName != SourceItemCDO->DisplayName)
		{
			continue;
		}

		const UAOFragment_SetStats* SourceStatsFrag = SourceItemCDO->FindFragmentByClass<UAOFragment_SetStats>();
		if (SourceStatsFrag == nullptr || !SourceStatsFrag->CanStack)
		{
			continue;
		}

		const int32 AvailableSpace = SourceStatsFrag->MaxStack - Entry.StackCount;
		if (AvailableSpace <= 0)
		{
			continue;
		}

		const int32 AddAmount = FMath::Min(ItemStackCount, AvailableSpace);
		Entry.StackCount += AddAmount;
		ItemStackCount -= AddAmount;
		bAddedAnything = true;
		InventoryList.MarkItemDirty(Entry);
	}

	while (ItemStackCount > 0)
	{
		const int32 TargetIndex = FindAvaliableSlot(TemplateInstance, ItemStackCount);
		if (TargetIndex == INDEX_NONE)
		{
			break;
		}

		FAOInventoryEntry& TargetEntry = InventoryList.Entries[TargetIndex];
		if (StatsFrag != nullptr && StatsFrag->CanStack)
		{
			const int32 AddAmount = FMath::Min(ItemStackCount, StatsFrag->MaxStack);
			TargetEntry.StackCount = AddAmount;
			TargetEntry.Instance = TemplateInstance;
			ItemStackCount -= AddAmount;
			bAddedAnything = true;

			if (FirstCreatedInstance == nullptr)
			{
				FirstCreatedInstance = TemplateInstance;
			}

			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && !bTemplateRegistered)
			{
				AddReplicatedSubObject(TemplateInstance);
				bTemplateRegistered = true;
			}

			InventoryList.MarkItemDirty(TargetEntry);
		}
		else
		{
			UAOInventoryItemInstance* UniqueInstance = CreateInventoryItemInstance(ItemClass, ItemDefClass);
			if (UniqueInstance == nullptr)
			{
				break;
			}

			TargetEntry.StackCount = 1;
			TargetEntry.Instance = UniqueInstance;
			--ItemStackCount;
			bAddedAnything = true;

			if (FirstCreatedInstance == nullptr)
			{
				FirstCreatedInstance = UniqueInstance;
			}

			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
			{
				AddReplicatedSubObject(UniqueInstance);
			}

			InventoryList.MarkItemDirty(TargetEntry);
		}
	}

	bCheck = bAddedAnything && ItemStackCount == 0;
	if (GetOwner()->HasAuthority() && bAddedAnything)
	{
		BroadCastInventoryChange();
	}

	return FirstCreatedInstance;
}

void UAOInventoryComponent::AddEntry(UAOInventoryItemInstance* Instance, int32& InCount, bool& bCheck)
{
	if (Instance == nullptr)
	{
		bCheck = false;
		return;
	}

	if (InventoryList.Entries.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("UAOInventoryComponent::AddEntry: InventoryList.Entries is empty."));
		bCheck = false;
		return;
	}

	AActor* OwningActor = GetOwner();
	check(OwningActor->HasAuthority());

	UAOInventoryItemDefinition* TargetItemCDO = Instance->GetItemCDO();
	if (TargetItemCDO == nullptr)
	{
		bCheck = false;
		return;
	}

	bool bAddedAnything = false;

	for (int32 SlotIndex = 0; SlotIndex < InventoryList.Entries.Num() && InCount > 0; ++SlotIndex)
	{
		FAOInventoryEntry& Entry = InventoryList.Entries[SlotIndex];
		if (Entry.Instance == nullptr)
		{
			continue;
		}

		UAOInventoryItemDefinition* SourceItemCDO = Entry.Instance->GetItemCDO();
		if (SourceItemCDO == nullptr || TargetItemCDO->DisplayName != SourceItemCDO->DisplayName)
		{
			continue;
		}

		const UAOFragment_SetStats* SourceStatsFrag = SourceItemCDO->FindFragmentByClass<UAOFragment_SetStats>();
		if (SourceStatsFrag == nullptr || !SourceStatsFrag->CanStack)
		{
			continue;
		}

		const int32 AvailableSpace = SourceStatsFrag->MaxStack - Entry.StackCount;
		if (AvailableSpace <= 0)
		{
			continue;
		}

		const int32 AddAmount = FMath::Min(InCount, AvailableSpace);
		Entry.StackCount += AddAmount;
		InCount -= AddAmount;
		bAddedAnything = true;
		InventoryList.MarkItemDirty(Entry);
	}

	const UAOFragment_SetStats* StatsFrag = TargetItemCDO->FindFragmentByClass<UAOFragment_SetStats>();
	while (InCount > 0)
	{
		const int32 TargetIndex = FindAvaliableSlot(Instance, InCount);
		if (TargetIndex == INDEX_NONE)
		{
			break;
		}

		FAOInventoryEntry& TargetEntry = InventoryList.Entries[TargetIndex];
		if (StatsFrag != nullptr && StatsFrag->CanStack)
		{
			const int32 AddAmount = FMath::Min(InCount, StatsFrag->MaxStack);
			TargetEntry.StackCount = AddAmount;
			TargetEntry.Instance = Instance;
			InCount -= AddAmount;
			bAddedAnything = true;
			InventoryList.MarkItemDirty(TargetEntry);
		}
		else
		{
			UAOInventoryItemInstance* TargetInstance =
				(InCount == 1) ? Instance : CreateInventoryItemInstance(Instance->GetClass(), Instance->ItemDef);
			if (TargetInstance == nullptr)
			{
				break;
			}

			TargetEntry.StackCount = 1;
			TargetEntry.Instance = TargetInstance;
			--InCount;
			bAddedAnything = true;

			if (TargetInstance != Instance && IsUsingRegisteredSubObjectList() && IsReadyForReplication())
			{
				AddReplicatedSubObject(TargetInstance);
			}

			InventoryList.MarkItemDirty(TargetEntry);
		}
	}

	bCheck = bAddedAnything && InCount == 0;
	if (GetOwner()->HasAuthority() && bAddedAnything)
	{
		BroadCastInventoryChange();
	}
}

int32 UAOInventoryComponent::FindAvaliableSlot(UAOInventoryItemInstance* Instance, int32 Count) const
{
	(void)Count;

	UAOInventoryItemDefinition* TargetItemCDO = Instance != nullptr ? Instance->GetItemCDO() : nullptr;
	const UAOFragment_SetStats* StatsFrag =
		TargetItemCDO != nullptr ? TargetItemCDO->FindFragmentByClass<UAOFragment_SetStats>() : nullptr;

	for (int32 SlotIndex = 0; SlotIndex < GetAllLists().Num(); ++SlotIndex)
	{
		if (GetAllLists()[SlotIndex].Instance == nullptr)
		{
			return SlotIndex;
		}
	}

	if (StatsFrag != nullptr && StatsFrag->CanStack && TargetItemCDO != nullptr)
	{
		for (int32 SlotIndex = 0; SlotIndex < GetAllLists().Num(); ++SlotIndex)
		{
			const FAOInventoryEntry& Entry = GetAllLists()[SlotIndex];
			if (Entry.Instance == nullptr)
			{
				continue;
			}

			UAOInventoryItemDefinition* ExistingDefinition = Entry.Instance->GetItemCDO();
			if (ExistingDefinition == nullptr || ExistingDefinition->DisplayName != TargetItemCDO->DisplayName)
			{
				continue;
			}

			if (Entry.StackCount < StatsFrag->MaxStack)
			{
				return SlotIndex;
			}
		}
	}

	return INDEX_NONE;
}

void UAOInventoryComponent::InitializeOrRefreshInventorySlots()
{
}

void UAOInventoryComponent::RemoveItemInstanceFromIndex(const int32 TargetIndex)
{
	InventoryList.Entries[TargetIndex] = FAOInventoryEntry();
	InventoryList.MarkArrayDirty();
}

void UAOInventoryComponent::InitializeParams()
{
}

void UAOInventoryComponent::RemoveEntry(UAOInventoryItemInstance* ItemInstance)
{
	for (auto EntryIt = InventoryList.Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FAOInventoryEntry& Entry = *EntryIt;
		if (Entry.Instance == ItemInstance)
		{
			Entry = FAOInventoryEntry();
			InventoryList.MarkArrayDirty();
		}
	}
}

inline TArray<UAOInventoryItemInstance*> UAOInventoryComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

inline TArray<FAOInventoryEntry> UAOInventoryComponent::GetAllLists() const
{
	return InventoryList.Entries;
}

TArray<UAOInventoryItemInstance*> FAOInventoryList::GetAllItems() const
{
	TArray<UAOInventoryItemInstance*> Results;
	Results.Reserve(Entries.Num());

	for (const FAOInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance != nullptr)
		{
			Results.Add(Entry.Instance);
		}
	}

	return Results;
}

void FAOInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Index : RemovedIndices)
	{
		FAOInventoryEntry& Stack = Entries[Index];
		Stack.LastObservedCount = 0;
	}

	OwnerComponent->BroadCastInventoryRemoveOnClient(RemovedIndices, FinalSize);
}

void FAOInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FAOInventoryEntry& Stack = Entries[Index];
		Stack.LastObservedCount = Stack.StackCount;
	}

	OwnerComponent->BroadCastInventoryAddOnClient(AddedIndices, FinalSize, Entries);
	OwnerComponent->OnInventoryObservedChanged.Broadcast();
}

void FAOInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Index : ChangedIndices)
	{
		FAOInventoryEntry& Stack = Entries[Index];
		check(Stack.LastObservedCount != INDEX_NONE);
		Stack.LastObservedCount = Stack.StackCount;
	}

	OwnerComponent->BroadCastInventoryChangeOnClient(ChangedIndices, FinalSize);
	OwnerComponent->OnInventoryObservedChanged.Broadcast();
}

void UAOInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, InventoryList);
}

void UAOInventoryComponent::AddItemInstance(UAOInventoryItemInstance* ItemInstance, int32& InCount, bool& bCheck)
{
	AddEntry(ItemInstance, InCount, bCheck);
	if (ItemInstance != nullptr)
	{
		ItemInstance->SetRuntimeOwnerActor(GetOwner());
	}
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance != nullptr)
	{
		AddReplicatedSubObject(ItemInstance);
	}
}

void UAOInventoryComponent::AddItemDefinition(
	TSubclassOf<UAOInventoryItemInstance> ItemClass,
	TSubclassOf<UAOInventoryItemDefinition> ItemDefClass,
	int32& StackCount,
	bool& bCheck)
{
	if (ItemDefClass != nullptr)
	{
		AddEntry(ItemClass, ItemDefClass, StackCount, bCheck);
	}
	else
	{
		bCheck = false;
	}
}

bool UAOInventoryComponent::CanFullyAcceptItemDefinitions(const TArray<FAOItemCatalogRow>& ItemRows, const TArray<int32>& ItemCounts) const
{
	if (ItemRows.Num() != ItemCounts.Num())
	{
		return false;
	}

	TArray<FSimulatedInventorySlot> SimulatedSlots;
	return SimulateAddItemDefinitionBatch(ItemRows, ItemCounts, SimulatedSlots);
}

bool UAOInventoryComponent::TryAddItemDefinitionsBatch(const TArray<FAOItemCatalogRow>& ItemRows, const TArray<int32>& ItemCounts)
{
	if (!CanFullyAcceptItemDefinitions(ItemRows, ItemCounts))
	{
		return false;
	}

	for (int32 Index = 0; Index < ItemRows.Num(); ++Index)
	{
		const FAOItemCatalogRow& ItemRow = ItemRows[Index];
		int32 StackCount = ItemCounts[Index];
		bool bAdded = false;
		AddItemDefinition(nullptr, ItemRow.ItemDefinitionClass, StackCount, bAdded);
		if (!bAdded || StackCount != 0)
		{
			return false;
		}
	}

	TArray<FAOInventoryAcquisitionMessage> AcquisitionMessages;
	BuildInventoryAcquisitionMessagesFromItemRows(ItemRows, ItemCounts, AcquisitionMessages);
	DispatchInventoryAcquisitionMessages(AcquisitionMessages);
	return true;
}

bool UAOInventoryComponent::CanFullyAcceptInventoryBatch(const FAOInventoryReceiveBatch& ReceiveBatch) const
{
	if (ReceiveBatch.IsEmpty())
	{
		return true;
	}

	TArray<FAOItemCatalogRow> ItemRows;
	TArray<int32> ItemCounts;
	ItemRows.Reserve(ReceiveBatch.DefinitionEntries.Num());
	ItemCounts.Reserve(ReceiveBatch.DefinitionEntries.Num());

	for (const FAOInventoryDefinitionEntry& DefinitionEntry : ReceiveBatch.DefinitionEntries)
	{
		if (DefinitionEntry.Count <= 0 || DefinitionEntry.ItemDefinitionClass == nullptr)
		{
			return false;
		}

		if (UAOInventoryItemDefinition::ResolveItemInstanceClass(
				DefinitionEntry.ItemDefinitionClass,
				DefinitionEntry.ItemInstanceClass) == nullptr)
		{
			return false;
		}

		FAOItemCatalogRow& ItemRow = ItemRows.AddDefaulted_GetRef();
		ItemRow.ItemDefinitionClass = DefinitionEntry.ItemDefinitionClass;
		ItemCounts.Add(DefinitionEntry.Count);
	}

	if (!CanFullyAcceptItemDefinitions(ItemRows, ItemCounts))
	{
		return false;
	}

	if (!ReceiveBatch.InstanceEntries.IsEmpty())
	{
		TArray<FSimulatedInventorySlot> SimulatedSlots;
		SimulatedSlots.Reserve(InventoryList.Entries.Num());

		for (const FAOInventoryEntry& Entry : InventoryList.Entries)
		{
			FSimulatedInventorySlot& SimulatedSlot = SimulatedSlots.AddDefaulted_GetRef();
			SimulatedSlot.ItemDefinition = Entry.Instance != nullptr ? Entry.Instance->GetItemCDO() : nullptr;
			SimulatedSlot.StackCount = Entry.StackCount;
		}

		if (!SimulateAddItemDefinitionBatch(ItemRows, ItemCounts, SimulatedSlots))
		{
			return false;
		}

		for (const FAOInventoryInstanceEntry& InstanceEntry : ReceiveBatch.InstanceEntries)
		{
			if (InstanceEntry.Count <= 0 || InstanceEntry.ItemInstance == nullptr)
			{
				return false;
			}

			const UAOInventoryItemDefinition* ItemDefinition = InstanceEntry.ItemInstance->GetItemCDO();
			if (ItemDefinition == nullptr)
			{
				return false;
			}

			int32 RemainingCount = InstanceEntry.Count;

			for (FSimulatedInventorySlot& SimulatedSlot : SimulatedSlots)
			{
				if (RemainingCount <= 0)
				{
					break;
				}

				if (SimulatedSlot.ItemDefinition == nullptr || SimulatedSlot.ItemDefinition->DisplayName != ItemDefinition->DisplayName)
				{
					continue;
				}

				SimulateAddItemDefinition(SimulatedSlot, *ItemDefinition, RemainingCount);
			}

			for (FSimulatedInventorySlot& SimulatedSlot : SimulatedSlots)
			{
				if (RemainingCount <= 0)
				{
					break;
				}

				if (SimulatedSlot.ItemDefinition != nullptr)
				{
					continue;
				}

				SimulateAddItemDefinition(SimulatedSlot, *ItemDefinition, RemainingCount);
			}

			if (RemainingCount > 0)
			{
				return false;
			}
		}
	}

	return true;
}

bool UAOInventoryComponent::TryAddInventoryBatch(const FAOInventoryReceiveBatch& ReceiveBatch)
{
	if (!CanFullyAcceptInventoryBatch(ReceiveBatch))
	{
		return false;
	}

	for (const FAOInventoryDefinitionEntry& DefinitionEntry : ReceiveBatch.DefinitionEntries)
	{
		int32 StackCount = DefinitionEntry.Count;
		bool bAdded = false;
		AddItemDefinition(DefinitionEntry.ItemInstanceClass, DefinitionEntry.ItemDefinitionClass, StackCount, bAdded);
		if (!bAdded || StackCount != 0)
		{
			return false;
		}
	}

	for (const FAOInventoryInstanceEntry& InstanceEntry : ReceiveBatch.InstanceEntries)
	{
		int32 StackCount = InstanceEntry.Count;
		bool bAdded = false;
		AddItemInstance(InstanceEntry.ItemInstance, StackCount, bAdded);
		if (!bAdded || StackCount != 0)
		{
			return false;
		}
	}

	TArray<FAOInventoryAcquisitionMessage> AcquisitionMessages;
	BuildInventoryAcquisitionMessagesFromReceiveBatch(ReceiveBatch, AcquisitionMessages);
	DispatchInventoryAcquisitionMessages(AcquisitionMessages);
	return true;
}

void UAOInventoryComponent::RemoveItemInstance(UAOInventoryItemInstance* ItemInstance)
{
	RemoveEntry(ItemInstance);

	if (ItemInstance != nullptr && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

bool UAOInventoryComponent::ConsumeItemAtSlot(int32 SlotIndex, int32 ConsumeCount)
{
	if (GetOwner() == nullptr || !GetOwner()->HasAuthority() || ConsumeCount <= 0 || !IsValidInventorySlotIndex(SlotIndex))
	{
		return false;
	}

	FAOInventoryEntry& Entry = InventoryList.Entries[SlotIndex];
	if (Entry.Instance == nullptr || Entry.StackCount < ConsumeCount)
	{
		return false;
	}

	Entry.StackCount -= ConsumeCount;
	if (Entry.StackCount > 0)
	{
		InventoryList.MarkItemDirty(Entry);
	}
	else
	{
		UAOInventoryItemInstance* ConsumedInstance = Entry.Instance;
		Entry = FAOInventoryEntry(this);
		InventoryList.MarkItemDirty(Entry);

		if (ConsumedInstance != nullptr && !IsItemInstanceReferencedByOtherSlots(ConsumedInstance, SlotIndex) && IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(ConsumedInstance);
		}
	}

	BroadCastInventoryChange(SlotIndex);
	return true;
}

bool UAOInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void UAOInventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (const FAOInventoryEntry& Entry : InventoryList.Entries)
		{
			if (UAOInventoryItemInstance* Instance = Entry.Instance)
			{
				if (IsValid(Instance))
				{
					AddReplicatedSubObject(Instance);
				}
			}
		}
	}
}

FAOInventoryEntry UAOInventoryComponent::FindInventoryEntryFromInstance(UAOInventoryItemInstance* ItemInstance) const
{
	for (const FAOInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance == ItemInstance)
		{
			return Entry;
		}
	}

	return FAOInventoryEntry();
}

int32 UAOInventoryComponent::FindInventorySlotIndexFromInstance(const UAOInventoryItemInstance* ItemInstance) const
{
	if (ItemInstance == nullptr)
	{
		return INDEX_NONE;
	}

	for (int32 SlotIndex = 0; SlotIndex < InventoryList.Entries.Num(); ++SlotIndex)
	{
		if (InventoryList.Entries[SlotIndex].Instance == ItemInstance)
		{
			return SlotIndex;
		}
	}

	return INDEX_NONE;
}

int32 UAOInventoryComponent::GetInventorySlotCount() const
{
	return InventoryList.Entries.Num();
}

const FAOInventoryEntry* UAOInventoryComponent::GetInventoryEntryAtSlot(int32 SlotIndex) const
{
	if (!IsValidInventorySlotIndex(SlotIndex))
	{
		return nullptr;
	}

	return &InventoryList.Entries[SlotIndex];
}

bool UAOInventoryComponent::IsValidInventorySlotIndex(int32 SlotIndex) const
{
	return InventoryList.Entries.IsValidIndex(SlotIndex);
}

bool UAOInventoryComponent::HasItemAtSlot(int32 SlotIndex) const
{
	return IsValidInventorySlotIndex(SlotIndex) && InventoryList.Entries[SlotIndex].Instance != nullptr;
}

bool UAOInventoryComponent::CanUseItemAtSlot(int32 SlotIndex, APawn* UserPawn) const
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	if (UserPawn == nullptr)
	{
		UserPawn = Cast<APawn>(GetOwner());
	}

	if (!IsValidInventorySlotIndex(SlotIndex) || UserPawn == nullptr)
	{
		return false;
	}

	const FAOInventoryEntry& Entry = InventoryList.Entries[SlotIndex];
	if (Entry.Instance == nullptr || Entry.StackCount <= 0 || Entry.Instance->GetItemCDO() == nullptr)
	{
		return false;
	}

	return Entry.Instance->CanUseFromInventory(Entry, UserPawn);
}

bool UAOInventoryComponent::CanExecuteExchangeRequest(
	UAOInventoryComponent* DraggedInventory,
	int32 DraggedSlotIndex,
	UAOInventoryComponent* DropInventory,
	int32 DropSlotIndex)
{
	if (DraggedInventory == nullptr || DropInventory == nullptr)
	{
		return false;
	}

	if (!DraggedInventory->IsValidInventorySlotIndex(DraggedSlotIndex)
		|| !DropInventory->IsValidInventorySlotIndex(DropSlotIndex))
	{
		return false;
	}

	const FAOInventoryEntry& DraggedEntry = DraggedInventory->InventoryList.Entries[DraggedSlotIndex];
	const FAOInventoryEntry& DropEntry = DropInventory->InventoryList.Entries[DropSlotIndex];

	// 先判断落点槽能不能接受被拖过来的条目。
	if (!DropInventory->CanAcceptInventoryEntryAtSlot(DraggedEntry, DropSlotIndex))
	{
		return false;
	}

	// 如果落点槽原本有东西，还要确认被换下来的旧条目能不能回到拖起侧原槽。
	if (DropEntry.Instance != nullptr && !DraggedInventory->CanAcceptInventoryEntryAtSlot(DropEntry, DraggedSlotIndex))
	{
		return false;
	}

	return true;
}

bool UAOInventoryComponent::ExecuteExchangeRequestOnAuthority(
	UAOInventoryComponent* DraggedInventory,
	int32 DraggedSlotIndex,
	UAOInventoryComponent* DropInventory,
	int32 DropSlotIndex)
{
	if (!CanExecuteExchangeRequest(DraggedInventory, DraggedSlotIndex, DropInventory, DropSlotIndex))
	{
		return false;
	}

	if ((DraggedInventory->GetOwner() == nullptr || !DraggedInventory->GetOwner()->HasAuthority())
		&& (DropInventory->GetOwner() == nullptr || !DropInventory->GetOwner()->HasAuthority()))
	{
		return false;
	}

	DraggedInventory->WhenItemExchange_Implementation(DropInventory, DraggedSlotIndex, DropSlotIndex);
	return true;
}

bool UAOInventoryComponent::ExecuteExchangeRequest(
	UAOInventoryComponent* DraggedInventory,
	int32 DraggedSlotIndex,
	UAOInventoryComponent* DropInventory,
	int32 DropSlotIndex)
{
	if (!CanExecuteExchangeRequest(DraggedInventory, DraggedSlotIndex, DropInventory, DropSlotIndex))
	{
		return false;
	}

	// 真正交换仍由被拖起侧发起 RPC。
	DraggedInventory->WhenItemExchange(DropInventory, DraggedSlotIndex, DropSlotIndex);
	return true;
}

bool UAOInventoryComponent::SimulateAddItemDefinition(
	FSimulatedInventorySlot& SimulatedSlot,
	const UAOInventoryItemDefinition& ItemDefinition,
	int32& InOutRemainingCount) const
{
	if (InOutRemainingCount <= 0)
	{
		return true;
	}

	const int32 MaxStackCount = GetMaxStackCountForDefinition(ItemDefinition);
	const bool bCanStack = MaxStackCount > 1;

	if (SimulatedSlot.ItemDefinition == nullptr)
	{
		const int32 AddedCount = FMath::Min(InOutRemainingCount, bCanStack ? MaxStackCount : 1);
		SimulatedSlot.ItemDefinition = &ItemDefinition;
		SimulatedSlot.StackCount = AddedCount;
		InOutRemainingCount -= AddedCount;
		return true;
	}

	if (!bCanStack || SimulatedSlot.ItemDefinition->DisplayName != ItemDefinition.DisplayName)
	{
		return false;
	}

	const int32 AvailableInStack = FMath::Max(0, MaxStackCount - SimulatedSlot.StackCount);
	const int32 AddedCount = FMath::Min(InOutRemainingCount, AvailableInStack);
	SimulatedSlot.StackCount += AddedCount;
	InOutRemainingCount -= AddedCount;
	return true;
}

bool UAOInventoryComponent::SimulateAddItemDefinitionBatch(
	const TArray<FAOItemCatalogRow>& ItemRows,
	const TArray<int32>& ItemCounts,
	TArray<FSimulatedInventorySlot>& SimulatedSlots) const
{
	if (ItemRows.Num() != ItemCounts.Num())
	{
		return false;
	}

	SimulatedSlots.Reset();
	SimulatedSlots.Reserve(InventoryList.Entries.Num());

	for (const FAOInventoryEntry& Entry : InventoryList.Entries)
	{
		FSimulatedInventorySlot& SimulatedSlot = SimulatedSlots.AddDefaulted_GetRef();
		SimulatedSlot.ItemDefinition = Entry.Instance != nullptr ? Entry.Instance->GetItemCDO() : nullptr;
		SimulatedSlot.StackCount = Entry.StackCount;
	}

	for (int32 Index = 0; Index < ItemRows.Num(); ++Index)
	{
		const FAOItemCatalogRow& ItemRow = ItemRows[Index];
		if (UAOInventoryItemDefinition::ResolveItemInstanceClass(ItemRow.ItemDefinitionClass) == nullptr)
		{
			return false;
		}

		const UAOInventoryItemDefinition* ItemDefinition = GetDefault<UAOInventoryItemDefinition>(ItemRow.ItemDefinitionClass);
		if (ItemDefinition == nullptr)
		{
			return false;
		}

		int32 RemainingCount = ItemCounts[Index];
		if (RemainingCount <= 0)
		{
			return false;
		}

		for (FSimulatedInventorySlot& SimulatedSlot : SimulatedSlots)
		{
			if (RemainingCount <= 0)
			{
				break;
			}

			if (SimulatedSlot.ItemDefinition == nullptr || SimulatedSlot.ItemDefinition->DisplayName != ItemDefinition->DisplayName)
			{
				continue;
			}

			SimulateAddItemDefinition(SimulatedSlot, *ItemDefinition, RemainingCount);
		}

		for (FSimulatedInventorySlot& SimulatedSlot : SimulatedSlots)
		{
			if (RemainingCount <= 0)
			{
				break;
			}

			if (SimulatedSlot.ItemDefinition != nullptr)
			{
				continue;
			}

			SimulateAddItemDefinition(SimulatedSlot, *ItemDefinition, RemainingCount);
		}

		if (RemainingCount > 0)
		{
			return false;
		}
	}

	return true;
}

int32 UAOInventoryComponent::GetMaxStackCountForDefinition(const UAOInventoryItemDefinition& ItemDefinition) const
{
	if (const UAOFragment_SetStats* StatsFragment = ItemDefinition.FindFragmentByClass<UAOFragment_SetStats>())
	{
		if (StatsFragment->CanStack)
		{
			return FMath::Max(1, StatsFragment->MaxStack);
		}
	}

	return 1;
}

bool UAOInventoryComponent::TryUseItemAtSlot(int32 SlotIndex, APawn* UserPawn)
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	if (UserPawn == nullptr)
	{
		UserPawn = Cast<APawn>(GetOwner());
	}

	if (!GetOwner()->HasAuthority())
	{
		ServerTryUseItemAtSlot(SlotIndex, UserPawn);
		return true;
	}

	if (!IsValidInventorySlotIndex(SlotIndex) || UserPawn == nullptr)
	{
		return false;
	}

	FAOInventoryEntry& Entry = InventoryList.Entries[SlotIndex];
	if (Entry.Instance == nullptr || Entry.StackCount <= 0 || Entry.Instance->GetItemCDO() == nullptr)
	{
		return false;
	}

	int32 ConsumeCount = 0;
	if (!Entry.Instance->TryUseFromInventory(Entry, UserPawn, ConsumeCount))
	{
		return false;
	}

	if (ConsumeCount > 0)
	{
		Entry.StackCount = FMath::Max(0, Entry.StackCount - ConsumeCount);
		if (Entry.StackCount > 0)
		{
			InventoryList.MarkItemDirty(Entry);
		}
		else
		{
			UAOInventoryItemInstance* ConsumedInstance = Entry.Instance;
			Entry = FAOInventoryEntry(this);
			InventoryList.MarkItemDirty(Entry);

			if (ConsumedInstance != nullptr && !IsItemInstanceReferencedByOtherSlots(ConsumedInstance, SlotIndex) && IsUsingRegisteredSubObjectList())
			{
				RemoveReplicatedSubObject(ConsumedInstance);
			}
		}

		BroadCastInventoryChange(SlotIndex);
	}

	return true;
}

bool UAOInventoryComponent::IsItemInstanceReferencedByOtherSlots(const UAOInventoryItemInstance* ItemInstance, int32 IgnoredSlotIndex) const
{
	if (ItemInstance == nullptr)
	{
		return false;
	}

	for (int32 SlotIndex = 0; SlotIndex < InventoryList.Entries.Num(); ++SlotIndex)
	{
		if (SlotIndex == IgnoredSlotIndex)
		{
			continue;
		}

		if (InventoryList.Entries[SlotIndex].Instance == ItemInstance)
		{
			return true;
		}
	}

	return false;
}

void UAOInventoryComponent::ServerTryUseItemAtSlot_Implementation(int32 SlotIndex, APawn* UserPawn)
{
	TryUseItemAtSlot(SlotIndex, UserPawn);
}

void UAOInventoryComponent::DispatchInventoryAcquisitionMessages(const TArray<FAOInventoryAcquisitionMessage>& Messages)
{
	if (Messages.IsEmpty())
	{
		return;
	}

	AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr || !OwningActor->HasAuthority())
	{
		return;
	}

	BroadcastInventoryAcquisitionMessagesLocally(Messages);

	if (!AOInventoryComponentPrivate::ShouldForwardAcquisitionMessagesToOwningClient(OwningActor))
	{
		return;
	}

	ClientBroadcastInventoryAcquisitionMessages(Messages);
}

void UAOInventoryComponent::BuildInventoryAcquisitionMessagesFromReceiveBatch(
	const FAOInventoryReceiveBatch& ReceiveBatch,
	TArray<FAOInventoryAcquisitionMessage>& OutMessages) const
{
	OutMessages.Reset();

	auto AppendOrMergeMessage = [&](TSubclassOf<UAOInventoryItemDefinition> ItemDefinitionClass, int32 Count)
	{
		if (ItemDefinitionClass == nullptr || Count <= 0)
		{
			return;
		}

		for (FAOInventoryAcquisitionMessage& ExistingMessage : OutMessages)
		{
			if (ExistingMessage.ItemDefinitionClass == ItemDefinitionClass)
			{
				ExistingMessage.Count += Count;
				return;
			}
		}

		FAOInventoryAcquisitionMessage& NewMessage = OutMessages.AddDefaulted_GetRef();
		NewMessage.Receiver = GetOwner();
		NewMessage.ItemDefinitionClass = ItemDefinitionClass;
		NewMessage.Count = Count;
	};

	for (const FAOInventoryDefinitionEntry& DefinitionEntry : ReceiveBatch.DefinitionEntries)
	{
		AppendOrMergeMessage(DefinitionEntry.ItemDefinitionClass, DefinitionEntry.Count);
	}

	for (const FAOInventoryInstanceEntry& InstanceEntry : ReceiveBatch.InstanceEntries)
	{
		const TSubclassOf<UAOInventoryItemDefinition> ItemDefinitionClass =
			InstanceEntry.ItemInstance != nullptr ? InstanceEntry.ItemInstance->ItemDef : nullptr;
		AppendOrMergeMessage(ItemDefinitionClass, InstanceEntry.Count);
	}
}

void UAOInventoryComponent::BuildInventoryAcquisitionMessagesFromItemRows(
	const TArray<FAOItemCatalogRow>& ItemRows,
	const TArray<int32>& ItemCounts,
	TArray<FAOInventoryAcquisitionMessage>& OutMessages) const
{
	OutMessages.Reset();

	if (ItemRows.Num() != ItemCounts.Num())
	{
		return;
	}

	for (int32 Index = 0; Index < ItemRows.Num(); ++Index)
	{
		const FAOItemCatalogRow& ItemRow = ItemRows[Index];
		const int32 Count = ItemCounts[Index];
		if (ItemRow.ItemDefinitionClass == nullptr || Count <= 0)
		{
			continue;
		}

		bool bMerged = false;
		for (FAOInventoryAcquisitionMessage& ExistingMessage : OutMessages)
		{
			if (ExistingMessage.ItemDefinitionClass == ItemRow.ItemDefinitionClass)
			{
				ExistingMessage.Count += Count;
				bMerged = true;
				break;
			}
		}

		if (!bMerged)
		{
			FAOInventoryAcquisitionMessage& NewMessage = OutMessages.AddDefaulted_GetRef();
			NewMessage.Receiver = GetOwner();
			NewMessage.ItemDefinitionClass = ItemRow.ItemDefinitionClass;
			NewMessage.Count = Count;
		}
	}
}

void UAOInventoryComponent::BroadcastInventoryAcquisitionMessagesLocally(const TArray<FAOInventoryAcquisitionMessage>& Messages) const
{
	if (UAOInventoryMessageSubsystem* InventoryMessageSubsystem = UAOInventoryMessageSubsystem::Get(this))
	{
		for (const FAOInventoryAcquisitionMessage& Message : Messages)
		{
			InventoryMessageSubsystem->BroadcastInventoryAcquisition(Message);
		}
	}
}

void UAOInventoryComponent::ClientBroadcastInventoryAcquisitionMessages_Implementation(const TArray<FAOInventoryAcquisitionMessage>& Messages)
{
	BroadcastInventoryAcquisitionMessagesLocally(Messages);
}
