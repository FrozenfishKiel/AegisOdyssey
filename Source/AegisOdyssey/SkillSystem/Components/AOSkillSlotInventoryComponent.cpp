// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/SkillSystem/Components/AOSkillSlotInventoryComponent.h"

#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "Components/GameFrameworkComponentManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillSlotInventoryComponent)

const FName UAOSkillSlotInventoryComponent::NAME_ActorFeatureName("SkillSlotInventory");

UAOSkillSlotInventoryComponent::UAOSkillSlotInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);

	// 技能槽库存适配层不应该参与世界统一入库。
	// 技能来源物品必须先存在于真实背包容器，再由玩家主动装配到技能槽。
	bAllowUnifiedInventoryIntake = false;
}

UAOSkillComponent* UAOSkillSlotInventoryComponent::GetOwningSkillComponent() const
{
	return UAOSkillComponent::FindSkillComponent(GetOwner());
}

void UAOSkillSlotInventoryComponent::SyncSkillRuntimeFromInventoryProjection()
{
	// 适配层的正式职责是把当前库存投影结果交回 SkillComponent，
	// 由 SkillComponent 继续维护技能实例、装配关系和授予结果。
	if (UAOSkillComponent* SkillComponent = GetOwningSkillComponent())
	{
		SkillComponent->SyncSkillSlotsFromInventoryProjection(InventoryList.Entries);
	}
}

void UAOSkillSlotInventoryComponent::BroadCastInventoryChange(int32 ChangedIndex)
{
	Super::BroadCastInventoryChange(ChangedIndex);

	// 每次技能槽库存投影发生变化后，都立刻把整份结果同步回 SkillComponent。
	// 这里故意不只同步单槽，因为技能槽之间的互换是整体状态变化，
	// 用整份投影同步能避免处理一半时出现的中间态误判。
	SyncSkillRuntimeFromInventoryProjection();
}

void UAOSkillSlotInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 和 QuickBar / SkillComponent 一样，把自己挂入统一初始化状态链。
	// 这里显式监听 SkillComponent 的初始化推进，确保后续如果初始化时序调整，
	// 技能槽库存适配层仍然能在正确时机重试自己的默认初始化链。
	BindOnActorInitStateChanged(UAOSkillComponent::NAME_ActorFeatureName, FGameplayTag(), false);
	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UAOSkillSlotInventoryComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();
}

void UAOSkillSlotInventoryComponent::InitializeParams()
{
	Super::InitializeParams();

	// 技能槽数量应当和 SkillComponent 的真实槽位数量保持一致。
	// 这里在初始化阶段主动对齐，避免蓝图/旧配置里还残留着别的 NumSlots。
	if (const UAOSkillComponent* SkillComponent = GetOwningSkillComponent())
	{
		NumSlots = SkillComponent->GetNumSkillSlots();
	}
}

void UAOSkillSlotInventoryComponent::InitializeOrRefreshInventorySlots()
{
	Super::InitializeOrRefreshInventorySlots();

	if (InventoryList.Entries.Num() < NumSlots)
	{
		InventoryList.Entries.Reserve(NumSlots);
		for (int32 SlotIndex = InventoryList.Entries.Num(); SlotIndex < NumSlots; ++SlotIndex)
		{
			FAOInventoryEntry Entry(this);
			InventoryList.Entries.Emplace(Entry);
		}

		InventoryList.MarkArrayDirty();
	}
}

void UAOSkillSlotInventoryComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	IGameFrameworkInitStateInterface::OnActorInitStateChanged(Params);

	// 技能库存适配层依赖 SkillComponent 作为运行时真相源。
	// 一旦 SkillComponent 状态推进，就再一次尝试推进自己的默认初始化链。
	if (Params.FeatureName == UAOSkillComponent::NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}
}

void UAOSkillSlotInventoryComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == AOGameplayTags::InitState_DataInitialized && DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		InitializeParams();

		// 技能槽库存投影不仅服务端要有，客户端本地也必须先把“有几个槽位”这层壳搭起来。
		// 否则就会出现一种很隐蔽但很致命的错位：
		// 1. SkillComponent / HUD 这边知道自己有 4 个技能槽，所以 UI 能画出 4 个格子；
		// 2. 但 SkillSlotInventoryComponent 本地如果还没补齐 4 个空槽，IsValidInventorySlotIndex() 就只会认少数已存在的条目；
		// 3. 最终拖拽落点在进入统一库存交换主链前，就会因为“目标槽位本地无效”被直接拦掉，
		//    表现出来就像只有第一个技能槽能放，后面槽位全都像失灵了一样。
		//
		// 所以这里正式改成：
		// - 服务端与客户端都先对齐本地投影槽位数量；
		// - 只有把投影结果翻译回 SkillComponent 运行时真相这一步，仍然坚持只由服务端执行。
		InitializeOrRefreshInventorySlots();

		if (HasAuthority())
		{
			SyncSkillRuntimeFromInventoryProjection();
		}
	}
}

void UAOSkillSlotInventoryComponent::CheckDefaultInitialization()
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
