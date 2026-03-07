// Fill out your copyright notice in the Description page of Project Settings.


#include "AOQuickBarComponent.h"
#include "NativeGameplayTags.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AegisOdyssey/Character/AOVMPawnComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "AegisOdyssey/Inventory/AOInventoryIteminstance.h"
#include "AegisOdyssey/Inventory/AOInventoryManagerComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOQuickBarComponent)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Input_QuickSlotSelect, "Input.QuickSlotSelect");
const FName UAOQuickBarComponent::NAME_ActorFeatureName("QuickBar");

UAOQuickBarComponent::UAOQuickBarComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UAOQuickBarComponent::BeginPlay()
{
	Super::BeginPlay();
	// Notifies state manager that we have spawned, then try rest of default initialization
	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));  //设定当前的CurrentState为InitState_Spawned然后开始调用一次状态链
}

void UAOQuickBarComponent::BroadCastInventoryChange()
{
	Super::BroadCastInventoryChange();
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

void UAOQuickBarComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	if (CurrentState == AOGameplayTags::InitState_DataInitialized && DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		
	}
}

void UAOQuickBarComponent::CheckDefaultInitialization()
{
	IGameFrameworkInitStateInterface::CheckDefaultInitialization();

	static const TArray<FGameplayTag> StateChain = { AOGameplayTags::InitState_Spawned, AOGameplayTags::InitState_DataAvailable,
AOGameplayTags::InitState_DataInitialized, AOGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UAOQuickBarComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();
}

void UAOQuickBarComponent::SetActivateIndex_Implementation(int32 NewIndex)
{
	if (InventoryList.Entries.IsValidIndex(NewIndex))
	{
		if (ActivateSlotIndex == NewIndex)
		{
			UnUseItemInSlot();

			OnRep_ActivateSlotIndex();

			ActivateSlotIndex = -1;
		}
		else
		{
			ActivateSlotIndex = NewIndex;
		
			UnUseItemInSlot();
		
			UseItemInSlot();

			OnRep_ActivateSlotIndex();
		}
	}
}
//使用物品
void UAOQuickBarComponent::UseItemInSlot()
{
    check(InventoryList.Entries.IsValidIndex(ActivateSlotIndex));
	if (UAOInventoryItemInstance* TargetItemInstance =  InventoryList.Entries[ActivateSlotIndex].Instance)
	{
		if (UAOInventoryManagerComponent* TargetInventoryManager = TargetItemInstance->FindTargetInventoryManager())
		{
			TargetInventoryManager->OnItemUse(InventoryList.Entries[ActivateSlotIndex]);
		}
	}
}
//取消使用物品
void UAOQuickBarComponent::UnUseItemInSlot()
{
	check(InventoryList.Entries.IsValidIndex(ActivateSlotIndex));
	if (UAOInventoryItemInstance* TargetItemInstance =  InventoryList.Entries[ActivateSlotIndex].Instance)
	{
		if (UAOInventoryManagerComponent* TargetInventoryManager = TargetItemInstance->FindTargetInventoryManager())
		{
			TargetInventoryManager->OnItemUnUse(InventoryList.Entries[ActivateSlotIndex]);
		}
	}
}

void UAOQuickBarComponent::InitializeParams()
{
	Super::InitializeParams();
	UAOVMPawnComponent* ViewModelComp = GetPawn<APawn>()->FindComponentByClass<UAOVMPawnComponent>();
	if (ViewModelComp)
	{
		ViewModelComp->CheckDefaultInitialization();
		//InventoryList.InventoryViewModel = ViewModelComp->GetCharacterInventoryViewModel();
	}
}

void UAOQuickBarComponent::InitializeOrRefreshInventorySlots()
{
	Super::InitializeOrRefreshInventorySlots();
//初始化背包格子
	if (GetOwner()->HasAuthority())
	{
		if (InventoryList.Entries.Num() < NumSlots)
		{
			InventoryList.Entries.Reserve(NumSlots);
			for (int32 i = 0; i < NumSlots; i++)
			{
				FAOInventoryEntry Entry(this);
				InventoryList.Entries.Emplace(Entry);
			}
			InventoryList.MarkArrayDirty();
		}
	}
}

void UAOQuickBarComponent::OnRep_ActivateSlotIndex()
{
    
}
void UAOQuickBarComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass , ActivateSlotIndex);
}
void UAOQuickBarComponent::Client_BroadCastInventoryAdd(const TArrayView<int32> AddIndices, int32 FinalSize, const TArray<FAOInventoryEntry>& TargetList)
{
	if (!AddIndices.IsEmpty())
	{

	}
}

void UAOQuickBarComponent::Client_BroadCastInventoryChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (!ChangedIndices.IsEmpty())
	{
		TArray<FAOInventoryEntry> ChangeRealList;
		for (int32 index = 0; index < ChangedIndices.Num(); index++)
		{
			ChangeRealList.Emplace(InventoryList.Entries[index]);
		}
		//InventoryList.InventoryViewModel->QuickBarDataChanged(ChangedIndices,FinalSize);
	}
}
void UAOQuickBarComponent::Client_BroadCastInventoryRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	if (!RemovedIndices.IsEmpty())
	{
		//InventoryList.InventoryViewModel->QuickBarDataRemove(RemovedIndices,FinalSize);
	}
}
