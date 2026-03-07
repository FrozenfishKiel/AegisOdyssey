// Fill out your copyright notice in the Description page of Project Settings.


#include "AOInventoryComponent.h"
#include "AOInventoryIteminstance.h"
#include "Net/UnrealNetwork.h"
#include "AOInventoryItemDefinition.h"
#include "Engine/ActorChannel.h"
#include "Fragments/AOFragment_SetStats.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryComponent)

UAOInventoryComponent::UAOInventoryComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
	,InventoryList(this)
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


void UAOInventoryComponent::WhenItemExchange_Implementation(UAOInventoryComponent* TargetItemContainer,
	const int32 SourceIndex, const int32 TargetIndex)
{
	if (!TargetItemContainer) return;
	FAOInventoryEntry& TargetItem = TargetItemContainer->InventoryList.Entries[TargetIndex];
	FAOInventoryEntry& SourceItem = InventoryList.Entries[SourceIndex];
	//获取两边的格子引用
	//如果放置的位置是个空格子
	if (SourceItem.Instance == nullptr)
	{
		FAOInventoryEntry TempEntry(TargetItemContainer);
		SourceItem = TargetItem;
		TargetItemContainer->InventoryList.Entries[TargetIndex] = TempEntry;
	}
	else
	{
		UAOInventoryItemInstance* TargetInstance = TargetItem.Instance;
		UAOInventoryItemInstance* SourceInstance = SourceItem.Instance;
		
		const UAOInventoryItemDefinition* SourceCDO = SourceInstance->GetItemCDO();
		const UAOInventoryItemDefinition* TargetCDO = TargetInstance->GetItemCDO();
		if (SourceCDO == nullptr || TargetCDO == nullptr) return; //我们要求双方必须有ItemCDO，这是规范化操作
		if (const UAOFragment_SetStats* SourceStatsFragment = SourceCDO->FindFragmentByClass<UAOFragment_SetStats>())
		{
			//优先检查是否可堆叠
			int32 TargetCount = TargetItem.StackCount;  //获取对方剩余数量
			int32 SourceCount = SourceItem.StackCount;  //获取当前物品剩余数量
			if (SourceItem == TargetItem)
			{
				if (SourceStatsFragment->CanStack)
				{
					if (SourceItem.StackCount == SourceStatsFragment->MaxStack) return;  //物品已满
					int32 SurPlus = SourceStatsFragment->MaxStack - SourceItem.StackCount;  //检查剩余值
					TargetCount -= SurPlus;  //若当前剩余20，而添加只有14，则可以添加，
					if (TargetCount <= 0)
					{
						FAOInventoryEntry TempEntry(TargetItem);
						SourceItem.StackCount += TargetItem.StackCount;
						TargetItem = TempEntry;
					}
					else
					{
						SourceItem.StackCount = FMath::Clamp(SourceItem.StackCount + TargetItem.StackCount, SourceItem.StackCount, SourceStatsFragment->MaxStack);
						TargetItem.StackCount = TargetCount;
					}
				}
				else
				{
					//同一个物品不同的堆叠只交换位置
					FAOInventoryEntry& TempEntry(SourceItem);
					SourceItem = TargetItem;
					TargetItem = TempEntry;
				}
			}
			else
			{
				//如果二者不同位置，交换位置
				FAOInventoryEntry& TempEntry = SourceItem;
				SourceItem = TargetItem;
				TargetItem = TempEntry;
			}
		}
	}
	if (GetOwner()->HasAuthority() || TargetItemContainer->GetOwner()->HasAuthority())
	{
		BroadCastInventoryChange();
		TargetItemContainer->BroadCastInventoryChange();
	}
	InventoryList.MarkItemDirty(SourceItem);
	TargetItemContainer->InventoryList.MarkItemDirty(TargetItem);
}


UAOInventoryItemInstance* UAOInventoryComponent::AddEntry(TSubclassOf<UAOInventoryItemInstance> ItemClass,
                                                          TSubclassOf<UAOInventoryItemDefinition> ItemDefClass,
                                                          int32& ItemStackCount, bool& bCheck)
{
	if (!ItemClass) return nullptr;
	if (InventoryList.Entries.IsEmpty()) 
	{
		UE_LOG(LogTemp, Error, TEXT("背包数据异常！！InventoryList.Entries为空"));
		return nullptr; //背包数据异常！！
	}

	AActor* OwningActor = GetOwner();
	check(OwningActor->HasAuthority());  ///确认当前添加背包的行为是服务器状态
	
	
	UAOInventoryItemInstance* Instance = NewObject<UAOInventoryItemInstance>(OwningActor,ItemClass);  //核心代码：构建一个新的Instance
	Instance->SetItemDef(ItemDefClass);
	UAOInventoryItemDefinition* TargetItemCDO = Instance->GetItemCDO();
	if (!TargetItemCDO) return nullptr;

	// 1. 优先堆叠已有物品
	for (int32 i = 0; i < InventoryList.Entries.Num() && ItemStackCount > 0; i++)
	{
		FAOInventoryEntry& Entry = InventoryList.Entries[i];
		if (!Entry.Instance) continue;
        
		UAOInventoryItemDefinition* SourceItemCDO = Entry.Instance->GetItemCDO();
		if (TargetItemCDO->DisplayName != SourceItemCDO->DisplayName) continue;

		const UAOFragment_SetStats* StatsFrag = SourceItemCDO->FindFragmentByClass<UAOFragment_SetStats>();

		if (!StatsFrag || !StatsFrag->CanStack) continue;

		// 计算可添加数量
		const int32 AvailableSpace = StatsFrag->MaxStack - Entry.StackCount;
		if (AvailableSpace <= 0) continue;  //当前已达到最大堆叠
        
		const int32 AddAmount = FMath::Min(ItemStackCount, AvailableSpace);  //当前位置剩余的数量与实际添加的数量取最小值
		Entry.StackCount += AddAmount;
		ItemStackCount -= AddAmount;
		InventoryList.MarkItemDirty(Entry);  //精确标记数组变化
	}

	// 2. 处理剩余数量（新堆叠或空位）
	while (ItemStackCount > 0)
	{
		int32 TargetIndex = FindAvaliableSlot(Instance, ItemStackCount);
		if (TargetIndex == -1)
		{
			bCheck = false;
			break;
		}// 无可用槽位
        
		FAOInventoryEntry& TargetEntry = InventoryList.Entries[TargetIndex];
		
		const UAOFragment_SetStats* StatsFrag = TargetItemCDO->FindFragmentByClass<UAOFragment_SetStats>();
        
		if (StatsFrag && StatsFrag->CanStack)
		{
			// 可堆叠物品
			const int32 AddAmount = FMath::Min(ItemStackCount, StatsFrag->MaxStack);
			TargetEntry.StackCount = AddAmount;
			TargetEntry.Instance = Instance;
			ItemStackCount -= AddAmount;
			InventoryList.MarkItemDirty(TargetEntry);  //精确标记数组变化
		}
		else
		{
			// 不可堆叠物品
			TargetEntry.StackCount = 1;
			TargetEntry.Instance = Instance;
			ItemStackCount--;
			InventoryList.MarkItemDirty(TargetEntry);  //精确标记数组变化
		}
	}
	bCheck = true;
	if (GetOwner()->HasAuthority())
	{
		BroadCastInventoryChange();
	}
	return Instance;
}

//收集已有的Instance。
void UAOInventoryComponent::AddEntry(UAOInventoryItemInstance* Instance, int32& InCount, bool& bCheck)
{
	if (!Instance) return;
	if (InventoryList.Entries.IsEmpty()) 
	{
		UE_LOG(LogTemp, Error, TEXT("背包数据异常！！InventoryList.Entries为空"));
		return; //背包数据异常！！
	}
    AActor* OwningActor = GetOwner();
    check(OwningActor->HasAuthority());

    UAOInventoryItemDefinition* TargetItemCDO = Instance->GetItemCDO();
    if (!TargetItemCDO) return;

    // 1. 优先堆叠已有物品
    for (int32 i = 0; i < InventoryList.Entries.Num() && InCount > 0; i++)
    {
        FAOInventoryEntry& Entry = InventoryList.Entries[i];
        if (!Entry.Instance) continue;
        
        UAOInventoryItemDefinition* SourceItemCDO = Entry.Instance->GetItemCDO();
        if (TargetItemCDO->DisplayName != SourceItemCDO->DisplayName) continue;

    	const UAOFragment_SetStats* StatsFrag = SourceItemCDO->FindFragmentByClass<UAOFragment_SetStats>();

        if (!StatsFrag || !StatsFrag->CanStack) continue;

        // 计算可添加数量
        const int32 AvailableSpace = StatsFrag->MaxStack - Entry.StackCount;
        if (AvailableSpace <= 0) continue;  //当前已达到最大堆叠
        
        const int32 AddAmount = FMath::Min(InCount, AvailableSpace);  //当前位置剩余的数量与实际添加的数量取最小值
        Entry.StackCount += AddAmount;
        InCount -= AddAmount;
    	InventoryList.MarkItemDirty(Entry);//精确标记数组变化
    }

    // 2. 处理剩余数量（新堆叠或空位）
    while (InCount > 0)
    {
        int32 TargetIndex = FindAvaliableSlot(Instance, InCount);
        if (TargetIndex == -1)
        {
        	bCheck = false;
        	break;
        }// 无可用槽位
        
        FAOInventoryEntry& TargetEntry = InventoryList.Entries[TargetIndex];
    	const UAOFragment_SetStats* StatsFrag = TargetItemCDO->FindFragmentByClass<UAOFragment_SetStats>();

        
        if (StatsFrag && StatsFrag->CanStack)
        {
            // 可堆叠物品
            const int32 AddAmount = FMath::Min(InCount, StatsFrag->MaxStack);
            TargetEntry.StackCount = AddAmount;
            TargetEntry.Instance = Instance;

            InCount -= AddAmount;
        	InventoryList.MarkItemDirty(TargetEntry);//精确标记数组变化

        }
        else
        {
            // 不可堆叠物品
            TargetEntry.StackCount = 1;
            TargetEntry.Instance = Instance;
        	InventoryList.Entries[TargetIndex] = TargetEntry;
            InCount--;
        	InventoryList.MarkItemDirty(TargetEntry);//精确标记数组变化

        }
    }
	bCheck = true;
	if (GetOwner()->HasAuthority())
	{
		BroadCastInventoryChange();
	}
}

int32 UAOInventoryComponent::FindAvaliableSlot(UAOInventoryItemInstance* Instance, int32 Count) const
{
	UAOInventoryItemDefinition* TargetItemCDO = Instance->GetItemCDO();
	const UAOFragment_SetStats* StatsFrag = TargetItemCDO ? 
		TargetItemCDO->FindFragmentByClass<UAOFragment_SetStats>() : nullptr;


	// 优先寻找空槽位
	for (int32 i = 0; i < GetAllLists().Num(); i++)
	{
		if (!GetAllLists()[i].Instance) return i;
	}

	// 可堆叠物品检查部分填充槽位
	if (StatsFrag && StatsFrag->CanStack)
	{
		for (int32 i = 0; i < GetAllLists().Num(); i++)
		{
			const FAOInventoryEntry& Entry = GetAllLists()[i];
			if (!Entry.Instance || Entry.Instance->GetItemCDO()->DisplayName != TargetItemCDO->DisplayName) 
				continue;

			if (Entry.StackCount < StatsFrag->MaxStack) return i;
		}
	}
	return -1; // 无可用槽位
}

// Server Call Function.
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

//移除对应的Instance
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
	TArray<UAOInventoryItemInstance* > Results;
	Results.Reserve(Entries.Num());  //预分配内存，防止在添加元素时先分配再添加，浪费性能
	
	for (const FAOInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance != nullptr)
		{
			Results.Add(Entry.Instance);
		}
	}
	return Results;
}

//复制序列规范：TArray允许复制序列中的某个对象，与常规TArray复制不同的是
//常规TArray复制的方式是服务器更改某个成员，都会直接同步所有的TArray对象到客户端
//而FastArray则只对某次发生改变的成员进行复制，防止多余的网络带宽占用，也加快了同步的速度
//则这三个函数都是更改的成员的索引序列
void FAOInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FAOInventoryEntry& Stack = Entries[Index];  // 记录服务器移除的Array的索引的对象
		Stack.LastObservedCount = 0;
	}
	OwnerComponent->Client_BroadCastInventoryRemove(RemovedIndices, FinalSize);
}

void FAOInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FAOInventoryEntry& Stack = Entries[Index];
		Stack.LastObservedCount = Stack.StackCount;
	}
	//OwnerComponent->Client_BroadCastInventoryAdd(AddedIndices,FinalSize, Entries);
}

void FAOInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FAOInventoryEntry& Stack = Entries[Index];
		check(Stack.LastObservedCount != INDEX_NONE);  //LastObservedCount是记录上一次网络更新的ArrayIndex
		Stack.LastObservedCount = Stack.StackCount;
	}
	OwnerComponent->Client_BroadCastInventoryChange(ChangedIndices, FinalSize);
}


void UAOInventoryComponent::GetLifetimeReplicatedProps(
	TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass,InventoryList);
}



void UAOInventoryComponent::AddItemInstance(UAOInventoryItemInstance* ItemInstance, int32& InCount, bool& bCheck)
{
	AddEntry(ItemInstance, InCount, bCheck);  //添加当前实例
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	}
}

void UAOInventoryComponent::AddItemDefinition(
	TSubclassOf<UAOInventoryItemInstance> ItemClass,TSubclassOf<UAOInventoryItemDefinition> ItemDefClass, int32& StackCount, bool& bCheck)
{
	UAOInventoryItemInstance* Result = nullptr;
	if (ItemDefClass != nullptr && ItemClass != nullptr)
	{
		Result = AddEntry(ItemClass,ItemDefClass, StackCount, bCheck);
		UAOInventoryItemDefinition* ItemDefinition = Result->GetItemCDO();
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			AddReplicatedSubObject(Result);
		}
	}
}

void UAOInventoryComponent::RemoveItemInstance(UAOInventoryItemInstance* ItemInstance)
{
	RemoveEntry(ItemInstance);  //删除当前实例

	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}


bool UAOInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (FAOInventoryEntry& Entry : InventoryList.Entries)
	{
		UAOInventoryItemInstance* Instance = Entry.Instance;
		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance , *Bunch, *RepFlags);
		}

	}
	return WroteSomething;
}

void UAOInventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	
	// Register existing ULyraInventoryItemInstance
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FAOInventoryEntry& Entry : InventoryList.Entries)
		{
			UAOInventoryItemInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

FAOInventoryEntry UAOInventoryComponent::FindInventoryEntryFromInstance(
	UAOInventoryItemInstance* ItemInstance) const
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
