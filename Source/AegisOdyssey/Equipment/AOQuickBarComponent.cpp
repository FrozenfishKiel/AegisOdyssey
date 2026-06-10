// Fill out your copyright notice in the Description page of Project Settings.


#include "AOQuickBarComponent.h"
#include "NativeGameplayTags.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "AegisOdyssey/Inventory/AOInventoryIteminstance.h"
#include "AegisOdyssey/Inventory/AOInventoryManagerComponent.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryMenu.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOQuickBarComponent)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Input_QuickSlotSelect, "Input.QuickSlotSelect");
const FName UAOQuickBarComponent::NAME_ActorFeatureName("QuickBar");

UAOQuickBarComponent::UAOQuickBarComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	ActivateSlotIndex = 0;

	// QuickBar 现在允许作为统一入包的前置尝试目标。
	// 真正的“先试 QuickBar，失败再回退到其他库存组件”选择逻辑仍然收口在 AOInventoryStatics。
	bAllowUnifiedInventoryIntake = true;
}

void UAOQuickBarComponent::BeginPlay()
{
	Super::BeginPlay();
	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));
	// 和背包同理，AI 的 PawnExt 初始化推进可能早于 QuickBar 自己的 BeginPlay。
	// 这里必须在打完 Spawned 后立刻补一次默认初始化，避免快捷栏槽位骨架没有真正建起来。
	CheckDefaultInitialization();
}

void UAOQuickBarComponent::BroadCastInventoryChange(const int32 ChangedIndex)
{
	Super::BroadCastInventoryChange();
	NotifyLocalQuickBarViewModelChanged();

	if (!InventoryList.Entries.IsValidIndex(ChangedIndex))
	{
		return;
	}

	UAOInventoryManagerComponent* TargetInventoryManager = nullptr;
	if (UAOInventoryItemInstance* TargetItemInstance = InventoryList.Entries[ChangedIndex].Instance)
	{
		TargetInventoryManager = TargetItemInstance->FindTargetInventoryManager();
	}
	else if (ChangedIndex == ActivateSlotIndex && GetOwner())
	{
		TargetInventoryManager = GetOwner()->FindComponentByClass<UAOInventoryManagerComponent>();
	}

	if (TargetInventoryManager)
	{
		TargetInventoryManager->ChangedItemOnSlot(ChangedIndex, ActivateSlotIndex, &InventoryList.Entries);
	}
}

void UAOQuickBarComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	IGameFrameworkInitStateInterface::OnActorInitStateChanged(Params);
	if (Params.FeatureName == UAOHeroComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == AOGameplayTags::InitState_DataInitialized)
		{
		}
	}
}

void UAOQuickBarComponent::HandleChangeInitState(
	UGameFrameworkComponentManager* Manager,
	FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	if (CurrentState == AOGameplayTags::InitState_DataInitialized && DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		InitializeParams();
		if (HasAuthority())
		{
			InitializeOrRefreshInventorySlots();
		}
	}
}

void UAOQuickBarComponent::CheckDefaultInitialization()
{
	IGameFrameworkInitStateInterface::CheckDefaultInitialization();

	static const TArray<FGameplayTag> StateChain = {
		AOGameplayTags::InitState_Spawned,
		AOGameplayTags::InitState_DataAvailable,
		AOGameplayTags::InitState_DataInitialized,
		AOGameplayTags::InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}

void UAOQuickBarComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();
}

void UAOQuickBarComponent::SetActivateIndex_Implementation(int32 NewIndex)
{
	UE_LOG(LogTemp, Log, TEXT("UAOQuickBarComponent::SetActivateIndex: NewIndex = %d"), NewIndex);

	if (!InventoryList.Entries.IsValidIndex(NewIndex))
	{
		return;
	}

	if (ActivateSlotIndex == NewIndex)
	{
		// 同一个槽位再次选中时，按重复使用当前槽位处理。
		UseItemInSlot(ActivateSlotIndex, ActivateSlotIndex);
		return;
	}

	const int32 OldIndex = ActivateSlotIndex;
	ActivateSlotIndex = NewIndex;

	UnUseItemInSlot(NewIndex, OldIndex);
	UseItemInSlot(OldIndex, ActivateSlotIndex);
}

void UAOQuickBarComponent::CycleActiveSlotForward()
{
	if (NumSlots <= 0)
	{
		return;
	}

	int32 NewIndex = 0;
	if (ActivateSlotIndex == INDEX_NONE)
	{
		NewIndex = 0;
	}
	else
	{
		NewIndex = ActivateSlotIndex + 1;
		if (NewIndex >= NumSlots)
		{
			NewIndex = 0;
		}
	}

	SetActivateIndex(NewIndex);
}

void UAOQuickBarComponent::CycleActiveSlotBackward()
{
	if (NumSlots <= 0)
	{
		return;
	}

	int32 NewIndex = NumSlots - 1;
	if (ActivateSlotIndex == INDEX_NONE)
	{
		NewIndex = NumSlots - 1;
	}
	else
	{
		NewIndex = ActivateSlotIndex - 1;
		if (NewIndex < 0)
		{
			NewIndex = NumSlots - 1;
		}
	}

	SetActivateIndex(NewIndex);
}

void UAOQuickBarComponent::UseItemInSlot(const int32 OldIndex, const int32 NewIndex)
{
	(void)OldIndex;
	check(InventoryList.Entries.IsValidIndex(NewIndex));
	if (UAOInventoryItemInstance* TargetItemInstance = InventoryList.Entries[NewIndex].Instance)
	{
		if (UAOInventoryManagerComponent* TargetInventoryManager = TargetItemInstance->FindTargetInventoryManager())
		{
			TargetInventoryManager->OnItemUse(InventoryList.Entries[NewIndex]);
		}
	}
}

void UAOQuickBarComponent::UnUseItemInSlot(const int32 NewIndex, const int32 OldIndex)
{
	(void)NewIndex;
	check(InventoryList.Entries.IsValidIndex(OldIndex));
	if (UAOInventoryItemInstance* TargetItemInstance = InventoryList.Entries[OldIndex].Instance)
	{
		if (UAOInventoryManagerComponent* TargetInventoryManager = TargetItemInstance->FindTargetInventoryManager())
		{
			TargetInventoryManager->OnItemUnUse(InventoryList.Entries[OldIndex]);
		}
	}
}

void UAOQuickBarComponent::InitializeParams()
{
	Super::InitializeParams();
	if (!QuickBarViewModel && GetOwner()->HasAuthority())
	{
		QuickBarViewModel = NewObject<UMVVM_InventoryMenu>(this);
		AddReplicatedSubObject(QuickBarViewModel);
		MARK_PROPERTY_DIRTY_FROM_NAME(UAOQuickBarComponent, QuickBarViewModel, this);
	}
}

void UAOQuickBarComponent::InitializeOrRefreshInventorySlots()
{
	Super::InitializeOrRefreshInventorySlots();

	// 初始化快捷栏槽位。
	if (InventoryList.Entries.Num() < NumSlots)
	{
		InventoryList.Entries.Reserve(NumSlots);
		for (int32 i = 0; i < NumSlots; i++)
		{
			FAOInventoryEntry Entry(this);
			InventoryList.Entries.Emplace(Entry);
		}
		InventoryList.MarkArrayDirty();
		NotifyLocalQuickBarViewModelChanged();
	}
}

void UAOQuickBarComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, ActivateSlotIndex);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, QuickBarViewModel, COND_None, REPNOTIFY_Always);
}

void UAOQuickBarComponent::BroadCastInventoryAddOnClient(const TArrayView<int32> AddIndices, int32 FinalSize, const TArray<FAOInventoryEntry>& TargetList)
{
	(void)AddIndices;
	(void)FinalSize;
	(void)TargetList;

	if (QuickBarViewModel)
	{
		QuickBarViewModel->OnQuickBarListChangedDynamic.Broadcast();
	}
}

void UAOQuickBarComponent::BroadCastInventoryChangeOnClient(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	(void)FinalSize;

	if (QuickBarViewModel)
	{
		QuickBarViewModel->OnQuickBarListChangedDynamic.Broadcast();
	}

	for (int32 Index : ChangedIndices)
	{
		if (ActivateSlotIndex == Index)
		{
			UseItemInSlot(ActivateSlotIndex, ActivateSlotIndex);
		}
	}
}

void UAOQuickBarComponent::BroadCastInventoryRemoveOnClient(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	(void)RemovedIndices;
	(void)FinalSize;

	if (QuickBarViewModel)
	{
		QuickBarViewModel->OnQuickBarListChangedDynamic.Broadcast();
	}
}

void UAOQuickBarComponent::OnRep_QuickBarViewModel()
{
}

void UAOQuickBarComponent::OnRep_ActivateSlotIndex(int32 LastActivateSlotIndex)
{
	if (LastActivateSlotIndex == ActivateSlotIndex)
	{
		if (InventoryList.Entries.IsValidIndex(ActivateSlotIndex))
		{
			UseItemInSlot(ActivateSlotIndex, ActivateSlotIndex);
		}
		return;
	}

	if (InventoryList.Entries.IsValidIndex(LastActivateSlotIndex))
	{
		UnUseItemInSlot(ActivateSlotIndex, LastActivateSlotIndex);
	}

	if (InventoryList.Entries.IsValidIndex(ActivateSlotIndex))
	{
		UseItemInSlot(LastActivateSlotIndex, ActivateSlotIndex);
	}
}

void UAOQuickBarComponent::NotifyLocalQuickBarViewModelChanged() const
{
	if (QuickBarViewModel)
	{
		QuickBarViewModel->OnQuickBarListChangedDynamic.Broadcast();
	}
}
