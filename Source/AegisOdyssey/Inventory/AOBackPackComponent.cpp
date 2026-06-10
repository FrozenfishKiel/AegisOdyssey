// Fill out your copyright notice in the Description page of Project Settings.


#include "AOBackPackComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "Net/Core/PushModel/PushModel.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryMenu.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOBackPackComponent)

const FName UAOBackPackComponent::NAME_ActorFeatureName("BackPack");

UAOBackPackComponent::UAOBackPackComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	// 当前玩家链路里，背包是世界奖励进入角色库存时的主接收容器。
	UnifiedInventoryIntakePriority = 100;
}

void UAOBackPackComponent::BroadCastInventoryChange(const int32 ChangedIndex)
{
	Super::BroadCastInventoryChange();
	NotifyLocalInventoryViewModelChanged();
}



//这个回调只会在本地触发，也就是说如果是服务器就在服务器触发，如果在客户端就在客户端触发，因为这个是由Instance判断调用的
void UAOBackPackComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	IGameFrameworkInitStateInterface::OnActorInitStateChanged(Params);
	if (Params.FeatureName == UAOHeroComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == AOGameplayTags::InitState_DataInitialized)
		{

		}
	}
}

//服务器权威触发
void UAOBackPackComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
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

void UAOBackPackComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();  //注册状态链，当EXt推进状态的时候会调用CheckDefaultInit
}

void UAOBackPackComponent::CheckDefaultInitialization()
{
	IGameFrameworkInitStateInterface::CheckDefaultInitialization();
	static const TArray<FGameplayTag> StateChain = { AOGameplayTags::InitState_Spawned, AOGameplayTags::InitState_DataAvailable,
	AOGameplayTags::InitState_DataInitialized, AOGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	//TryToChangeInitState(AOGameplayTags::InitState_Spawned);
	
	ContinueInitStateChain(StateChain);
}

void UAOBackPackComponent::BeginPlay()
{
	Super::BeginPlay();
	// Notifies state manager that we have spawned, then try rest of default initialization
	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));
	// AI 出生链里，PawnExt 可能会比背包组件更早尝试推进整条初始化状态链。
	// 这里只打 Spawned 不继续补一次默认初始化的话，背包就可能永久卡在“已出生但未就绪”，
	// 最终表现成 UI 看不到任何背包槽位。
	CheckDefaultInitialization();
}

void UAOBackPackComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass,BackPackViewModel,COND_None,REPNOTIFY_Always);
}


void UAOBackPackComponent::InitializeOrRefreshInventorySlots()
{
	Super::InitializeOrRefreshInventorySlots();
	//初始化背包格子
	if (InventoryList.Entries.Num() < NumSlots)
	{
		InventoryList.Entries.Reserve(NumSlots);
		for (int32 i = 0; i < NumSlots; i++)
		{
			FAOInventoryEntry Entry(this);
			InventoryList.Entries.Emplace(Entry);
		}
		InventoryList.MarkArrayDirty();
		NotifyLocalInventoryViewModelChanged();
	}
}

void UAOBackPackComponent::InitializeParams()
{
	Super::InitializeParams();
	if (!BackPackViewModel && GetOwner()->HasAuthority())
	{
		BackPackViewModel = NewObject<UMVVM_InventoryMenu>();
		AddReplicatedSubObject(BackPackViewModel);
		MARK_PROPERTY_DIRTY_FROM_NAME(UAOBackPackComponent,BackPackViewModel,this);  //标记为脏，触发客户端回调
	}
}
void UAOBackPackComponent::BroadCastInventoryAddOnClient(const TArrayView<int32> AddIndices, int32 FinalSize, const TArray<FAOInventoryEntry>& TargetList)
{
	BackPackViewModel->OnInventoryListChangedDynamic.Broadcast();
}

void UAOBackPackComponent::BroadCastInventoryChangeOnClient(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	BackPackViewModel->OnInventoryListChangedDynamic.Broadcast();
}

void UAOBackPackComponent::BroadCastInventoryRemoveOnClient(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	BackPackViewModel->OnInventoryListChangedDynamic.Broadcast();
}

void UAOBackPackComponent::OnRep_BackPackViewModel()
{
	//InitializeOrRefreshInventorySlots();
}

void UAOBackPackComponent::NotifyLocalInventoryViewModelChanged() const
{
	if (BackPackViewModel)
	{
		BackPackViewModel->OnInventoryListChangedDynamic.Broadcast();
	}
}
