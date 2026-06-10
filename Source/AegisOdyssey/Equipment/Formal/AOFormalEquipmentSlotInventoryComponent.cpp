#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentSlotInventoryComponent.h"

#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryMenu.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOFormalEquipmentSlotInventoryComponent)

const FName UAOFormalEquipmentSlotInventoryComponent::NAME_ActorFeatureName("FormalEquipmentInventory");

UAOFormalEquipmentSlotInventoryComponent::UAOFormalEquipmentSlotInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);

	// 正式装备槽不是“统一入库口”。
	// 正式装备必须先存在于真实库存中，再由玩家主动拖入或使用后装备到正式槽。
	bAllowUnifiedInventoryIntake = false;
}

UAOFormalEquipmentManagerComponent* UAOFormalEquipmentSlotInventoryComponent::GetOwningFormalEquipmentManager() const
{
	return UAOFormalEquipmentManagerComponent::FindFormalEquipmentManagerComponent(GetOwner());
}

void UAOFormalEquipmentSlotInventoryComponent::SyncFormalEquipmentRuntimeFromInventoryProjection()
{
	// 每次正式装备槽投影变化后，都把整份槽位快照交回正式装备管理组件。
	// 由运行时真相组件决定 EquippedInstance 如何切换，以及 AbilitySet 如何授予 / 回收。
	if (UAOFormalEquipmentManagerComponent* FormalEquipmentManager = GetOwningFormalEquipmentManager())
	{
		FormalEquipmentManager->SyncFormalEquipmentFromInventoryProjection(InventoryList.Entries);
	}
}

void UAOFormalEquipmentSlotInventoryComponent::BroadCastInventoryChange(int32 ChangedIndex)
{
	Super::BroadCastInventoryChange(ChangedIndex);

	NotifyLocalFormalEquipmentViewModelChanged();

	// 这里只要库存投影变了，就立刻回推一次运行时真相。
	// 正式装备之间的替换会同时影响装备实例和属性效果，拖到后面批量同步更容易留下中间态。
	SyncFormalEquipmentRuntimeFromInventoryProjection();
}

bool UAOFormalEquipmentSlotInventoryComponent::CanAcceptInventoryEntryAtSlot(const FAOInventoryEntry& IncomingEntry, int32 TargetSlotIndex) const
{
	if (IncomingEntry.Instance == nullptr)
	{
		// 允许空条目进入，统一交换主链在做“替换回源”时会依赖这条语义。
		return true;
	}

	if (const UAOFormalEquipmentManagerComponent* FormalEquipmentManager = GetOwningFormalEquipmentManager())
	{
		// 正式装备槽不自己展开判断“是不是头盔 / 盔甲”，而是转交给正式装备管理组件做统一运行时校验。
		return FormalEquipmentManager->CanAcceptInventoryItemForFormalSlot(IncomingEntry.Instance, TargetSlotIndex);
	}

	return false;
}

void UAOFormalEquipmentSlotInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 正式装备槽的初始化依赖正式装备管理组件的初始化推进。
	// 所以这里显式监听它的状态变化，保证初始化时序变化后仍能重试自己的默认初始化链。
	BindOnActorInitStateChanged(UAOFormalEquipmentManagerComponent::NAME_ActorFeatureName, FGameplayTag(), false);
	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UAOFormalEquipmentSlotInventoryComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();
}

void UAOFormalEquipmentSlotInventoryComponent::InitializeParams()
{
	Super::InitializeParams();

	// 正式装备栏这一轮固定就是五槽，不随外部配置漂移。
	NumSlots = 5;

	if (FormalEquipmentViewModel == nullptr && GetOwner() != nullptr && GetOwner()->HasAuthority())
	{
		// 表现层复用当前库存菜单 ViewModel。
		// 这里只是单独走一份“正式装备五槽快照”，避免把正式装备栏硬塞进背包或快捷栏数据面里。
		FormalEquipmentViewModel = NewObject<UMVVM_InventoryMenu>(this);
		AddReplicatedSubObject(FormalEquipmentViewModel);
		MARK_PROPERTY_DIRTY_FROM_NAME(UAOFormalEquipmentSlotInventoryComponent, FormalEquipmentViewModel, this);
	}

	if (FormalEquipmentViewModel != nullptr)
	{
		FormalEquipmentViewModel->SetFormalEquipmentList(InventoryList.Entries);
	}
}

void UAOFormalEquipmentSlotInventoryComponent::InitializeOrRefreshInventorySlots()
{
	Super::InitializeOrRefreshInventorySlots();

	if (InventoryList.Entries.Num() < NumSlots)
	{
		// 客户端本地也必须先补齐五个空槽，否则 UI 会看到“外观存在，但底层目标槽索引无效”的错位状态，
		// 拖拽在进入统一交换主链前就会被提前拦下。
		InventoryList.Entries.Reserve(NumSlots);
		for (int32 SlotIndex = InventoryList.Entries.Num(); SlotIndex < NumSlots; ++SlotIndex)
		{
			FAOInventoryEntry Entry(this);
			InventoryList.Entries.Emplace(Entry);
		}

		InventoryList.MarkArrayDirty();
		NotifyLocalFormalEquipmentViewModelChanged();
	}
}

void UAOFormalEquipmentSlotInventoryComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	IGameFrameworkInitStateInterface::OnActorInitStateChanged(Params);
	if (Params.FeatureName == UAOFormalEquipmentManagerComponent::NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}
}

void UAOFormalEquipmentSlotInventoryComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == AOGameplayTags::InitState_DataInitialized && DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		InitializeParams();
		InitializeOrRefreshInventorySlots();

		// 正式装备槽投影依赖正式装备管理组件的初始化推进。
		// 所以这里显式监听它的状态变化，保证初始化时序变化后仍能重试自己的默认初始化链。
		if (HasAuthority())
		{
			SyncFormalEquipmentRuntimeFromInventoryProjection();
		}
	}
}

void UAOFormalEquipmentSlotInventoryComponent::CheckDefaultInitialization()
{
	IGameFrameworkInitStateInterface::CheckDefaultInitialization();

	static const TArray<FGameplayTag> StateChain =
	{
		AOGameplayTags::InitState_Spawned,
		AOGameplayTags::InitState_DataAvailable,
		AOGameplayTags::InitState_DataInitialized,
		AOGameplayTags::InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}

void UAOFormalEquipmentSlotInventoryComponent::BroadCastInventoryAddOnClient(const TArrayView<int32> AddIndices, int32 FinalSize, const TArray<FAOInventoryEntry>& TargetList)
{
	(void)AddIndices;
	(void)FinalSize;
	(void)TargetList;

	if (FormalEquipmentViewModel != nullptr)
	{
		FormalEquipmentViewModel->SetFormalEquipmentList(InventoryList.Entries);
	}
}

void UAOFormalEquipmentSlotInventoryComponent::BroadCastInventoryChangeOnClient(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	(void)ChangedIndices;
	(void)FinalSize;

	if (FormalEquipmentViewModel != nullptr)
	{
		FormalEquipmentViewModel->SetFormalEquipmentList(InventoryList.Entries);
	}
}

void UAOFormalEquipmentSlotInventoryComponent::BroadCastInventoryRemoveOnClient(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	(void)RemovedIndices;
	(void)FinalSize;

	if (FormalEquipmentViewModel != nullptr)
	{
		FormalEquipmentViewModel->SetFormalEquipmentList(InventoryList.Entries);
	}
}

void UAOFormalEquipmentSlotInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, FormalEquipmentViewModel, COND_None, REPNOTIFY_Always);
}

void UAOFormalEquipmentSlotInventoryComponent::OnRep_FormalEquipmentViewModel()
{
	NotifyLocalFormalEquipmentViewModelChanged();
}

void UAOFormalEquipmentSlotInventoryComponent::NotifyLocalFormalEquipmentViewModelChanged() const
{
	if (FormalEquipmentViewModel != nullptr)
	{
		FormalEquipmentViewModel->SetFormalEquipmentList(InventoryList.Entries);
	}
}
