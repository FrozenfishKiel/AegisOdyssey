// Fill out your copyright notice in the Description page of Project Settings.


#include "AOBackPackComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AegisOdyssey/Character/AOVMPawnComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOBackPackComponent)

const FName UAOBackPackComponent::NAME_ActorFeatureName("BackPack");

UAOBackPackComponent::UAOBackPackComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UAOBackPackComponent::BroadCastInventoryChange()
{
	Super::BroadCastInventoryChange();

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
}


void UAOBackPackComponent::InitializeOrRefreshInventorySlots()
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

void UAOBackPackComponent::InitializeParams()
{
	Super::InitializeParams();
	UAOVMPawnComponent* ViewModelComp = GetPawn<APawn>()->FindComponentByClass<UAOVMPawnComponent>();
	if (ViewModelComp)
	{
		ViewModelComp->CheckDefaultInitialization();  //尝试初始化一次VMPawn
		//InventoryList.InventoryViewModel = ViewModelComp->GetCharacterInventoryViewModel();
	}
}
void UAOBackPackComponent::Client_BroadCastInventoryAdd(const TArrayView<int32> AddIndices, int32 FinalSize, const TArray<FAOInventoryEntry>& TargetList)
{
	if (!AddIndices.IsEmpty())
	{
		//InventoryList.InventoryViewModel->InventoryListDataAdd(AddIndices,FinalSize);
	}
}

void UAOBackPackComponent::Client_BroadCastInventoryChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (!ChangedIndices.IsEmpty())
	{
		TArray<FAOInventoryEntry> ChangedRealList;
		for (int32 index = 0; index < ChangedIndices.Num(); index++)
		{
			ChangedRealList.Emplace(InventoryList.Entries[index]);
		}
		//InventoryList.InventoryViewModel->InventoryListDataChanged(ChangedIndices,FinalSize);
	}
}

void UAOBackPackComponent::Client_BroadCastInventoryRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	if (!RemovedIndices.IsEmpty())
	{
		//InventoryList.InventoryViewModel->InventoryListDataRemove(RemovedIndices,FinalSize);
	}
}
