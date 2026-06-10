// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"

#include "AegisOdyssey/SkillSystem/Core/AOSkillDefinition.h"
#include "AegisOdyssey/SkillSystem/Core/AOSkillInstance.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_SkillSource.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillComponent)

const FName UAOSkillComponent::NAME_ActorFeatureName("SkillSystem");

UAOSkillComponent::UAOSkillComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 技能组件需要在网络上保持统一的槽位和实例状态，因此默认开启复制。
	SetIsReplicatedByDefault(true);

	// SkillInstance 作为运行时子对象存在，沿用项目里 Inventory 的注册式子对象复制方式。
	bReplicateUsingRegisteredSubObjectList = true;
}

bool UAOSkillComponent::IsValidSkillSlotIndex(int32 SlotIndex) const
{
	return SkillSlots.IsValidIndex(SlotIndex);
}

TArray<FAOSkillSlotViewData> UAOSkillComponent::GetSkillSlotViewDataList() const
{
	TArray<FAOSkillSlotViewData> Result;
	Result.Reserve(SkillSlots.Num());

	for (const FAOSkillSlotEntry& SlotEntry : SkillSlots)
	{
		FAOSkillSlotViewData ViewData;
		BuildSkillSlotViewData(SlotEntry, ViewData);
		Result.Add(ViewData);
	}

	return Result;
}

bool UAOSkillComponent::GetSkillSlotViewData(int32 SlotIndex, FAOSkillSlotViewData& OutViewData) const
{
	if (!IsValidSkillSlotIndex(SlotIndex))
	{
		return false;
	}

	return BuildSkillSlotViewData(SkillSlots[SlotIndex], OutViewData);
}

TArray<FAOEquippedSkillViewData> UAOSkillComponent::GetEquippedSkillViewDataList() const
{
	TArray<FAOEquippedSkillViewData> Result;
	Result.Reserve(EquippedSkillInstances.Num());

	for (const TObjectPtr<UAOSkillInstance>& SkillInstance : EquippedSkillInstances)
	{
		FAOEquippedSkillViewData ViewData;
		if (BuildEquippedSkillViewData(SkillInstance, ViewData))
		{
			Result.Add(ViewData);
		}
	}

	return Result;
}

bool UAOSkillComponent::GetEquippedSkillViewData(const UAOSkillInstance* SkillInstance, FAOEquippedSkillViewData& OutViewData) const
{
	return BuildEquippedSkillViewData(SkillInstance, OutViewData);
}

bool UAOSkillComponent::IsSkillSlotInputTag(const FGameplayTag& InputTag) const
{
	// 这里判断的不是标签名字像不像 SkillSlot，
	// 而是当前角色的槽位配置里是否真的把它当成了技能槽输入。
	return FindSkillSlotIndexByInputTag(InputTag) != INDEX_NONE;
}

int32 UAOSkillComponent::FindSkillSlotIndexByInputTag(const FGameplayTag& InputTag) const
{
	if (!InputTag.IsValid())
	{
		return INDEX_NONE;
	}

	for (const FAOSkillSlotEntry& SlotEntry : SkillSlots)
	{
		if (SlotEntry.InputTag == InputTag)
		{
			return SlotEntry.SlotIndex;
		}
	}

	return INDEX_NONE;
}

bool UAOSkillComponent::InjectSkillSlotInputCommand(FGameplayTag InputTag, TEnumAsByte<EInputType> InputType) const
{
	// 统一先把输入标签翻译成槽位索引，再走按槽位触发的正式入口。
	return InjectSkillSlotInputCommandByIndex(FindSkillSlotIndexByInputTag(InputTag), InputType);
}

bool UAOSkillComponent::InjectSkillSlotInputCommandByIndex(int32 SlotIndex, TEnumAsByte<EInputType> InputType) const
{
	// 这条链只做技能层语义校验，真正的输入桥仍然复用 Hero 那一层。
	if (!IsValidSkillSlotIndex(SlotIndex))
	{
		return false;
	}

	const FAOSkillSlotEntry& SlotEntry = SkillSlots[SlotIndex];
	if (!SlotEntry.InputTag.IsValid() || !SlotEntry.SkillInstance)
	{
		return false;
	}

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	UAOHeroComponent* HeroComponent = UAOHeroComponent::FindHeroComponent(OwnerActor);
	if (!HeroComponent)
	{
		return false;
	}

	return HeroComponent->InjectAbilityInputCommand(SlotEntry.InputTag, InputType);
}

bool UAOSkillComponent::ExecuteSkillSlotCommand(FGameplayTag InputTag, TEnumAsByte<EInputType> InputType) const
{
	return ExecuteSkillSlotCommandByIndex(FindSkillSlotIndexByInputTag(InputTag), InputType);
}

bool UAOSkillComponent::ExecuteSkillSlotCommandByIndex(int32 SlotIndex, TEnumAsByte<EInputType> InputType) const
{
	// 这条链服务 StateTree / AI，直接走 SkillComponent -> ASC，
	// 不再回灌 Hero 输入广播，避免再次形成语义回路。
	if (!IsValidSkillSlotIndex(SlotIndex))
	{
		return false;
	}

	const FAOSkillSlotEntry& SlotEntry = SkillSlots[SlotIndex];
	if (!SlotEntry.InputTag.IsValid() || !SlotEntry.SkillInstance || !SlotEntry.GrantedAbilitySpecHandle.IsValid())
	{
		return false;
	}

	UAOAbilitySystem* AbilitySystem = GetAbilitySystemComponent();
	if (!AbilitySystem)
	{
		return false;
	}

	switch (InputType.GetValue())
	{
	case EInputType::Trigger:
		AbilitySystem->AbilityInputTagPressed(SlotEntry.InputTag);
		break;

	case EInputType::Start:
		AbilitySystem->AbilityInputTagStarted(SlotEntry.InputTag);
		break;

	case EInputType::Release:
		AbilitySystem->AbilityInputTagReleased(SlotEntry.InputTag);
		break;

	default:
		return false;
	}

	const UWorld* World = GetWorld();
	AbilitySystem->ProcessAbilityInput(0.0f, World ? World->IsPaused() : false);
	return true;
}

bool UAOSkillComponent::GetSkillInstanceCooldownState(const UAOSkillInstance* SkillInstance, float& OutTimeRemaining, float& OutTotalDuration) const
{
	OutTimeRemaining = 0.0f;
	OutTotalDuration = 0.0f;

	if (!SkillInstance)
	{
		return false;
	}

	FGameplayTagContainer CooldownTags;
	SkillInstance->GetCooldownIdentityTags(CooldownTags);
	return QueryCooldownStateFromTags(CooldownTags, OutTimeRemaining, OutTotalDuration);
}

bool UAOSkillComponent::GetSkillSlotCooldownState(int32 SlotIndex, float& OutTimeRemaining, float& OutTotalDuration) const
{
	OutTimeRemaining = 0.0f;
	OutTotalDuration = 0.0f;

	if (!IsValidSkillSlotIndex(SlotIndex))
	{
		return false;
	}

	const UAOSkillInstance* SkillInstance = SkillSlots[SlotIndex].SkillInstance;
	return GetSkillInstanceCooldownState(SkillInstance, OutTimeRemaining, OutTotalDuration);
}

bool UAOSkillComponent::IsSkillInstanceOnCooldown(const UAOSkillInstance* SkillInstance) const
{
	float TimeRemaining = 0.0f;
	float TotalDuration = 0.0f;
	return GetSkillInstanceCooldownState(SkillInstance, TimeRemaining, TotalDuration);
}

bool UAOSkillComponent::IsSkillSlotOnCooldown(int32 SlotIndex) const
{
	float TimeRemaining = 0.0f;
	float TotalDuration = 0.0f;
	return GetSkillSlotCooldownState(SlotIndex, TimeRemaining, TotalDuration);
}

float UAOSkillComponent::GetSkillInstanceCooldownRemaining(const UAOSkillInstance* SkillInstance) const
{
	float TimeRemaining = 0.0f;
	float TotalDuration = 0.0f;
	GetSkillInstanceCooldownState(SkillInstance, TimeRemaining, TotalDuration);
	return TimeRemaining;
}

float UAOSkillComponent::GetSkillSlotCooldownRemaining(int32 SlotIndex) const
{
	float TimeRemaining = 0.0f;
	float TotalDuration = 0.0f;
	GetSkillSlotCooldownState(SlotIndex, TimeRemaining, TotalDuration);
	return TimeRemaining;
}

UAOSkillInstance* UAOSkillComponent::CreateSkillInstance(UAOSkillDefinition* SkillDefinition, UAOInventoryItemInstance* SourceItemInstance)
{
	// 当前方案只允许服务端创建 SkillInstance，保证“来源 -> 实例 -> 装配”归属稳定。
	if (!GetOwner() || !GetOwner()->HasAuthority() || !SkillDefinition)
	{
		return nullptr;
	}

	UAOSkillInstance* SkillInstance = NewObject<UAOSkillInstance>(this);
	if (!SkillInstance)
	{
		return nullptr;
	}

	SkillInstance->InitializeSkillInstance(SkillDefinition, SourceItemInstance);
	return SkillInstance;
}

bool UAOSkillComponent::EquipSkillInstanceToSlot(UAOSkillInstance* SkillInstance, int32 SlotIndex)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !CanEquipSkillInstanceToSlot(SkillInstance, SlotIndex))
	{
		return false;
	}

	FAOSkillSlotEntry& SlotEntry = SkillSlots[SlotIndex];
	if (SlotEntry.SkillInstance == SkillInstance)
	{
		const bool bRefreshResult = RefreshGrantedAbilityForSlot(SlotIndex);
		if (bRefreshResult)
		{
			NotifySkillObservationChanged();
		}
		return bRefreshResult;
	}

	int32 ExistingEquippedSlotIndex = INDEX_NONE;
	for (int32 ExistingSlotIndex = 0; ExistingSlotIndex < SkillSlots.Num(); ++ExistingSlotIndex)
	{
		if (ExistingSlotIndex == SlotIndex)
		{
			continue;
		}

		if (SkillSlots[ExistingSlotIndex].SkillInstance == SkillInstance)
		{
			ExistingEquippedSlotIndex = ExistingSlotIndex;
			break;
		}
	}

	if (ExistingEquippedSlotIndex != INDEX_NONE)
	{
		// 同一实例被重新装到另一个技能槽时，本质上是跨槽移动。
		FAOSkillSlotEntry& ExistingSlotEntry = SkillSlots[ExistingEquippedSlotIndex];
		RevokeSkillFromSlot(ExistingSlotEntry);
		ExistingSlotEntry.ResetRuntimeBinding();
	}

	if (UAOSkillInstance* ExistingSkillInstance = SlotEntry.SkillInstance)
	{
		// 目标槽已有旧实例时，严格按槽位语义做完整替换。
		RevokeSkillFromSlot(SlotEntry);
		ExistingSkillInstance->SetEquippedState(false, INDEX_NONE);
		RemoveSkillInstanceFromEquippedSet(ExistingSkillInstance);
		UnregisterSkillInstanceSubObject(ExistingSkillInstance);
		DestroySkillInstanceIfUnused(ExistingSkillInstance);
	}

	SlotEntry.SkillInstance = SkillInstance;
	SkillInstance->SetEquippedState(true, SlotIndex);
	EquippedSkillInstances.AddUnique(SkillInstance);
	RegisterSkillInstanceSubObject(SkillInstance);

	const bool bRefreshResult = RefreshGrantedAbilityForSlot(SlotIndex);
	if (bRefreshResult)
	{
		NotifySkillObservationChanged();
	}

	return bRefreshResult;
}

bool UAOSkillComponent::UnequipSkillFromSlot(int32 SlotIndex)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !IsValidSkillSlotIndex(SlotIndex))
	{
		return false;
	}

	FAOSkillSlotEntry& SlotEntry = SkillSlots[SlotIndex];
	UAOSkillInstance* ExistingSkillInstance = SlotEntry.SkillInstance;
	if (!ExistingSkillInstance)
	{
		return false;
	}

	RevokeSkillFromSlot(SlotEntry);
	ExistingSkillInstance->SetEquippedState(false, INDEX_NONE);
	RemoveSkillInstanceFromEquippedSet(ExistingSkillInstance);
	UnregisterSkillInstanceSubObject(ExistingSkillInstance);
	SlotEntry.ResetRuntimeBinding();
	DestroySkillInstanceIfUnused(ExistingSkillInstance);
	NotifySkillObservationChanged();
	return true;
}

UAOSkillInstance* UAOSkillComponent::GetOrCreateSkillInstanceFromSourceItem(UAOInventoryItemInstance* SourceItemInstance)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !SourceItemInstance)
	{
		return nullptr;
	}

	if (UAOSkillInstance* ExistingSkillInstance = FindSkillInstanceBySourceItem(SourceItemInstance))
	{
		return ExistingSkillInstance;
	}

	UAOSkillDefinition* SkillDefinition = FindSkillDefinitionFromSourceItem(SourceItemInstance);
	if (!SkillDefinition)
	{
		return nullptr;
	}

	return CreateSkillInstance(SkillDefinition, SourceItemInstance);
}

bool UAOSkillComponent::EquipSkillSourceItemToSlot(UAOInventoryItemInstance* SourceItemInstance, int32 SlotIndex)
{
	UAOSkillInstance* SkillInstance = GetOrCreateSkillInstanceFromSourceItem(SourceItemInstance);
	if (!SkillInstance)
	{
		return false;
	}

	return EquipSkillInstanceToSlot(SkillInstance, SlotIndex);
}

bool UAOSkillComponent::RemoveSkillInstanceForSourceItem(UAOInventoryItemInstance* SourceItemInstance)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !SourceItemInstance)
	{
		return false;
	}

	UAOSkillInstance* SkillInstance = FindSkillInstanceBySourceItem(SourceItemInstance);
	if (!SkillInstance)
	{
		return false;
	}

	if (IsSkillInstanceEquipped(SkillInstance))
	{
		return false;
	}

	return DestroySkillInstanceIfUnused(SkillInstance);
}

void UAOSkillComponent::SyncSkillSlotsFromInventoryProjection(const TArray<FAOInventoryEntry>& Slots)
{
	// 这是“技能槽库存投影 -> 技能运行时真相”的正式翻译入口。
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (Slots.Num() < SkillSlots.Num())
	{
		return;
	}

	TMap<UAOInventoryItemInstance*, int32> DesiredItemToSlotMap;
	TArray<UAOInventoryItemInstance*> DesiredItemsBySlot;
	DesiredItemsBySlot.SetNum(SkillSlots.Num());

	for (int32 SlotIndex = 0; SlotIndex < SkillSlots.Num(); ++SlotIndex)
	{
		UAOInventoryItemInstance* SourceItemInstance = Slots[SlotIndex].Instance;
		if (!SourceItemInstance)
		{
			DesiredItemsBySlot[SlotIndex] = nullptr;
			continue;
		}

		if (!IsSkillSourceItem(SourceItemInstance))
		{
			return;
		}

		if (DesiredItemToSlotMap.Contains(SourceItemInstance))
		{
			return;
		}

		DesiredItemToSlotMap.Add(SourceItemInstance, SlotIndex);
		DesiredItemsBySlot[SlotIndex] = SourceItemInstance;
	}

	bool bAnySlotChanged = false;
	TArray<TObjectPtr<UAOSkillInstance>> PendingCleanupInstances;
	TMap<UAOInventoryItemInstance*, TObjectPtr<UAOSkillInstance>> DetachedInstancesBySourceItem;

	for (int32 SlotIndex = 0; SlotIndex < SkillSlots.Num(); ++SlotIndex)
	{
		FAOSkillSlotEntry& SlotEntry = SkillSlots[SlotIndex];
		UAOSkillInstance* ExistingSkillInstance = SlotEntry.SkillInstance;
		if (!ExistingSkillInstance)
		{
			continue;
		}

		UAOInventoryItemInstance* ExistingSourceItem = ExistingSkillInstance->GetSourceItemInstance();
		const int32* DesiredSlotIndexPtr = DesiredItemToSlotMap.Find(ExistingSourceItem);
		if (DesiredSlotIndexPtr && *DesiredSlotIndexPtr == SlotIndex)
		{
			continue;
		}

		RevokeSkillFromSlot(SlotEntry);
		ExistingSkillInstance->SetEquippedState(false, INDEX_NONE);
		RemoveSkillInstanceFromEquippedSet(ExistingSkillInstance);
		bAnySlotChanged = true;

		if (ExistingSourceItem)
		{
			DetachedInstancesBySourceItem.Add(ExistingSourceItem, ExistingSkillInstance);
		}

		SlotEntry.ResetRuntimeBinding();
		PendingCleanupInstances.AddUnique(ExistingSkillInstance);
	}

	for (int32 SlotIndex = 0; SlotIndex < SkillSlots.Num(); ++SlotIndex)
	{
		FAOSkillSlotEntry& SlotEntry = SkillSlots[SlotIndex];
		UAOInventoryItemInstance* DesiredSourceItem = DesiredItemsBySlot[SlotIndex];

		if (!DesiredSourceItem)
		{
			continue;
		}

		UAOSkillInstance* SkillInstance = nullptr;
		if (const TObjectPtr<UAOSkillInstance>* DetachedSkillInstancePtr = DetachedInstancesBySourceItem.Find(DesiredSourceItem))
		{
			SkillInstance = DetachedSkillInstancePtr->Get();
		}

		if (!SkillInstance)
		{
			SkillInstance = FindSkillInstanceBySourceItem(DesiredSourceItem);
		}

		if (!SkillInstance)
		{
			SkillInstance = GetOrCreateSkillInstanceFromSourceItem(DesiredSourceItem);
		}

		if (!SkillInstance)
		{
			continue;
		}

		if (SlotEntry.SkillInstance != SkillInstance)
		{
			bAnySlotChanged = true;
		}

		SlotEntry.SkillInstance = SkillInstance;
		SkillInstance->SetEquippedState(true, SlotIndex);
		EquippedSkillInstances.AddUnique(SkillInstance);
		RegisterSkillInstanceSubObject(SkillInstance);
	}

	for (int32 SlotIndex = 0; SlotIndex < SkillSlots.Num(); ++SlotIndex)
	{
		RefreshGrantedAbilityForSlot(SlotIndex);
	}

	for (UAOSkillInstance* PendingCleanupInstance : PendingCleanupInstances)
	{
		if (!PendingCleanupInstance || IsSkillInstanceEquipped(PendingCleanupInstance))
		{
			continue;
		}

		UnregisterSkillInstanceSubObject(PendingCleanupInstance);
		DestroySkillInstanceIfUnused(PendingCleanupInstance);
	}

	if (bAnySlotChanged)
	{
		NotifySkillObservationChanged();
	}
}

bool UAOSkillComponent::CanEquipSkillInstanceToSlot(const UAOSkillInstance* SkillInstance, int32 SlotIndex, bool bAllowReplace) const
{
	if (!SkillInstance || !IsValidSkillSlotIndex(SlotIndex))
	{
		return false;
	}

	if (!SkillInstance->GetSkillDefinition())
	{
		return false;
	}

	const FAOSkillSlotEntry& SlotEntry = SkillSlots[SlotIndex];
	if (!bAllowReplace && SlotEntry.SkillInstance != nullptr && SlotEntry.SkillInstance != SkillInstance)
	{
		return false;
	}

	// 同一 SkillInstance 允许从旧槽位移动到新槽位，
	// 但最终仍然只能落在一个槽位上。
	return true;
}

bool UAOSkillComponent::IsSkillInstanceEquipped(const UAOSkillInstance* SkillInstance) const
{
	return SkillInstance != nullptr && EquippedSkillInstances.Contains(SkillInstance);
}

UAOSkillInstance* UAOSkillComponent::FindSkillInstanceBySourceItem(const UAOInventoryItemInstance* SourceItemInstance) const
{
	// 当前不维护独立的“未装配技能库”，所以优先从已装配集合里反查现存实例。
	if (!SourceItemInstance)
	{
		return nullptr;
	}

	for (const TObjectPtr<UAOSkillInstance>& SkillInstance : EquippedSkillInstances)
	{
		if (SkillInstance && SkillInstance->GetSourceItemInstance() == SourceItemInstance)
		{
			return SkillInstance;
		}
	}

	return nullptr;
}

UAOSkillDefinition* UAOSkillComponent::FindSkillDefinitionFromSourceItem(const UAOInventoryItemInstance* SourceItemInstance) const
{
	if (!SourceItemInstance)
	{
		return nullptr;
	}

	const UAOInventoryItemDefinition* ItemDefinition = SourceItemInstance->GetItemCDO();
	if (!ItemDefinition)
	{
		return nullptr;
	}

	if (const UAOFragment_SkillSource* SkillSourceFragment = ItemDefinition->FindFragmentByClass<UAOFragment_SkillSource>())
	{
		return SkillSourceFragment->SkillDefinition;
	}

	return nullptr;
}

bool UAOSkillComponent::IsSkillSourceItem(const UAOInventoryItemInstance* SourceItemInstance) const
{
	// 这里只回答“它是不是技能来源物品”。
	return FindSkillDefinitionFromSourceItem(SourceItemInstance) != nullptr;
}

bool UAOSkillComponent::CanAcceptSourceItemForSkillSlot(const UAOInventoryItemInstance* SourceItemInstance, int32 SlotIndex) const
{
	// 这里只回答“一个真实物品能不能进入这个技能槽”，
	// 不负责普通库存空格的那套泛化交换语义。
	if (!SourceItemInstance || !IsValidSkillSlotIndex(SlotIndex))
	{
		return false;
	}

	if (!IsSkillSourceItem(SourceItemInstance))
	{
		return false;
	}

	if (const UAOSkillInstance* ExistingSkillInstance = FindSkillInstanceBySourceItem(SourceItemInstance))
	{
		return CanEquipSkillInstanceToSlot(ExistingSkillInstance, SlotIndex);
	}

	return true;
}

bool UAOSkillComponent::RequestEquipSourceItemToSlot(UAOInventoryItemInstance* SourceItemInstance, int32 SlotIndex)
{
	if (!SourceItemInstance || !IsValidSkillSlotIndex(SlotIndex))
	{
		return false;
	}

	if (!CanAcceptSourceItemForSkillSlot(SourceItemInstance, SlotIndex))
	{
		return false;
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		return EquipSkillSourceItemToSlot(SourceItemInstance, SlotIndex);
	}

	ServerRequestEquipSourceItemToSlot(SourceItemInstance, SlotIndex);
	return true;
}

void UAOSkillComponent::ServerRequestEquipSourceItemToSlot_Implementation(UAOInventoryItemInstance* SourceItemInstance, int32 SlotIndex)
{
	if (!CanAcceptSourceItemForSkillSlot(SourceItemInstance, SlotIndex))
	{
		return;
	}

	EquipSkillSourceItemToSlot(SourceItemInstance, SlotIndex);
}

void UAOSkillComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	IGameFrameworkInitStateInterface::OnActorInitStateChanged(Params);

	// Hero 只是输入广播源，一旦它初始化推进，就再尝试补绑一次输入委托。
	if (Params.FeatureName == UAOHeroComponent::NAME_ActorFeatureName)
	{
		BindHeroInputDelegates();
	}
}

void UAOSkillComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == AOGameplayTags::InitState_DataInitialized && DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		InitializeParams();
		InitializeOrRefreshSkillSlots();
		BindHeroInputDelegates();

		if (HasAuthority())
		{
			// 进入 GameplayReady 后，把现有槽位结果补刷到 ASC。
			for (int32 SlotIndex = 0; SlotIndex < SkillSlots.Num(); ++SlotIndex)
			{
				RefreshGrantedAbilityForSlot(SlotIndex);
			}
		}
	}
}

void UAOSkillComponent::CheckDefaultInitialization()
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

void UAOSkillComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();
}

void UAOSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	BindOnActorInitStateChanged(UAOHeroComponent::NAME_ActorFeatureName, FGameplayTag(), false);
	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));
	BindHeroInputDelegates();
	CheckDefaultInitialization();
}

void UAOSkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindHeroInputDelegates();
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void UAOSkillComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (!IsUsingRegisteredSubObjectList())
	{
		return;
	}

	for (const TObjectPtr<UAOSkillInstance>& SkillInstance : EquippedSkillInstances)
	{
		if (IsValid(SkillInstance))
		{
			AddReplicatedSubObject(SkillInstance);
		}
	}
}

void UAOSkillComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, SkillSlots);
	DOREPLIFETIME(ThisClass, EquippedSkillInstances);
}

void UAOSkillComponent::OnRep_SkillSlots()
{
	NotifySkillObservationChanged();
}

void UAOSkillComponent::OnRep_EquippedSkillInstances()
{
	NotifySkillObservationChanged();
}

void UAOSkillComponent::InitializeParams()
{
	// 默认把技能槽输入语义固定成 Input.SkillSlot1~4。
	if (DefaultSlotInputTags.Num() < NumSkillSlots)
	{
		DefaultSlotInputTags.SetNum(NumSkillSlots);
	}

	if (NumSkillSlots > 0 && !DefaultSlotInputTags[0].IsValid())
	{
		DefaultSlotInputTags[0] = AOGameplayTags::Input_SkillSlot1;
	}

	if (NumSkillSlots > 1 && !DefaultSlotInputTags[1].IsValid())
	{
		DefaultSlotInputTags[1] = AOGameplayTags::Input_SkillSlot2;
	}

	if (NumSkillSlots > 2 && !DefaultSlotInputTags[2].IsValid())
	{
		DefaultSlotInputTags[2] = AOGameplayTags::Input_SkillSlot3;
	}

	if (NumSkillSlots > 3 && !DefaultSlotInputTags[3].IsValid())
	{
		DefaultSlotInputTags[3] = AOGameplayTags::Input_SkillSlot4;
	}
}

void UAOSkillComponent::InitializeOrRefreshSkillSlots()
{
	// 这里只补齐槽位壳体，不动已有运行时装配结果。
	const int32 ExistingNum = SkillSlots.Num();
	if (ExistingNum >= NumSkillSlots)
	{
		return;
	}

	SkillSlots.Reserve(NumSkillSlots);
	for (int32 SlotIndex = ExistingNum; SlotIndex < NumSkillSlots; ++SlotIndex)
	{
		FAOSkillSlotEntry NewSlotEntry(SlotIndex);
		if (DefaultSlotInputTags.IsValidIndex(SlotIndex))
		{
			NewSlotEntry.InputTag = DefaultSlotInputTags[SlotIndex];
		}

		SkillSlots.Add(NewSlotEntry);
	}

	NotifySkillObservationChanged();
}

void UAOSkillComponent::BindHeroInputDelegates()
{
	if (BoundHeroComponent.IsValid()
		&& OnHeroInputPressedHandle.IsValid()
		&& OnHeroInputStartedHandle.IsValid()
		&& OnHeroInputReleasedHandle.IsValid())
	{
		return;
	}

	UnbindHeroInputDelegates();

	UAOHeroComponent* HeroComponent = UAOHeroComponent::FindHeroComponent(GetOwner());
	if (!HeroComponent)
	{
		return;
	}

	BoundHeroComponent = HeroComponent;
	OnHeroInputPressedHandle = HeroComponent->OnPressInputLoad.Add(FOnPressInputLoad::FDelegate::CreateUObject(this, &ThisClass::HandleHeroInputPressed));
	OnHeroInputStartedHandle = HeroComponent->OnStartInputLoad.Add(FOnStartInputLoad::FDelegate::CreateUObject(this, &ThisClass::HandleHeroInputStarted));
	OnHeroInputReleasedHandle = HeroComponent->OnReleaseInputLoad.Add(FOnReleaseInputLoad::FDelegate::CreateUObject(this, &ThisClass::HandleHeroInputReleased));
}

void UAOSkillComponent::UnbindHeroInputDelegates()
{
	UAOHeroComponent* HeroComponent = BoundHeroComponent.Get();
	if (HeroComponent)
	{
		if (OnHeroInputPressedHandle.IsValid())
		{
			HeroComponent->OnPressInputLoad.Remove(OnHeroInputPressedHandle);
		}

		if (OnHeroInputStartedHandle.IsValid())
		{
			HeroComponent->OnStartInputLoad.Remove(OnHeroInputStartedHandle);
		}

		if (OnHeroInputReleasedHandle.IsValid())
		{
			HeroComponent->OnReleaseInputLoad.Remove(OnHeroInputReleasedHandle);
		}
	}

	OnHeroInputPressedHandle.Reset();
	OnHeroInputStartedHandle.Reset();
	OnHeroInputReleasedHandle.Reset();
	BoundHeroComponent.Reset();
}

void UAOSkillComponent::HandleHeroInputPressed(FGameplayTag InputTag, EInputType InputType)
{
	HandleObservedHeroInput(InputTag, InputType);
}

void UAOSkillComponent::HandleHeroInputStarted(FGameplayTag InputTag, EInputType InputType)
{
	HandleObservedHeroInput(InputTag, InputType);
}

void UAOSkillComponent::HandleHeroInputReleased(FGameplayTag InputTag, EInputType InputType)
{
	HandleObservedHeroInput(InputTag, InputType);
}

void UAOSkillComponent::HandleObservedHeroInput(FGameplayTag InputTag, EInputType InputType)
{
	// 这里是技能层的“订阅后自己判断”边界：
	// 1. 先判断 Hero 广播出来的输入是否属于技能槽语义；
	// 2. 再判断该槽当前是否真的装有技能；
	// 3. 当前阶段这里只保留观察边界，不再把输入重复送回 ASC。
	const int32 SlotIndex = FindSkillSlotIndexByInputTag(InputTag);
	if (!IsValidSkillSlotIndex(SlotIndex))
	{
		return;
	}

	const FAOSkillSlotEntry& SlotEntry = SkillSlots[SlotIndex];
	if (!SlotEntry.SkillInstance)
	{
		return;
	}

	(void)InputType;
}

void UAOSkillComponent::RegisterSkillInstanceSubObject(UAOSkillInstance* SkillInstance)
{
	if (!SkillInstance || !IsUsingRegisteredSubObjectList() || !IsReadyForReplication())
	{
		return;
	}

	AddReplicatedSubObject(SkillInstance);
}

void UAOSkillComponent::UnregisterSkillInstanceSubObject(UAOSkillInstance* SkillInstance)
{
	if (!SkillInstance || !IsUsingRegisteredSubObjectList())
	{
		return;
	}

	RemoveReplicatedSubObject(SkillInstance);
}

void UAOSkillComponent::RemoveSkillInstanceFromEquippedSet(UAOSkillInstance* SkillInstance)
{
	if (!SkillInstance)
	{
		return;
	}

	EquippedSkillInstances.Remove(SkillInstance);
}

bool UAOSkillComponent::DestroySkillInstanceIfUnused(UAOSkillInstance* SkillInstance)
{
	// 当前不维护“拥有但未装配”的独立技能实例库。
	// 只要实例已经不在任何槽位中，就允许退出当前运行时管理。
	if (!SkillInstance || IsSkillInstanceEquipped(SkillInstance))
	{
		return false;
	}

	UnregisterSkillInstanceSubObject(SkillInstance);
	return true;
}

UAOAbilitySystem* UAOSkillComponent::GetAbilitySystemComponent() const
{
	const APawn* OwnerPawn = GetPawn<APawn>();
	if (!OwnerPawn)
	{
		return nullptr;
	}

	if (const UAOExtPawnComponent* ExtPawnComponent = UAOExtPawnComponent::FindAOExtPawnComponent(OwnerPawn))
	{
		return ExtPawnComponent->GetAOAbilitySystemComponent();
	}

	return nullptr;
}

bool UAOSkillComponent::QueryCooldownStateFromTags(const FGameplayTagContainer& CooldownTags, float& OutTimeRemaining, float& OutTotalDuration) const
{
	OutTimeRemaining = 0.0f;
	OutTotalDuration = 0.0f;

	// 第五阶段开始，技能冷却的真实来源是 ASC 上仍然存活的冷却效果，
	// 而不是槽位上的临时字段。
	UAOAbilitySystem* AbilitySystem = GetAbilitySystemComponent();
	if (!AbilitySystem || CooldownTags.IsEmpty())
	{
		return false;
	}

	const FGameplayEffectQuery CooldownQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
	const TArray<TPair<float, float>> TimeRemainingAndDuration = AbilitySystem->GetActiveEffectsTimeRemainingAndDuration(CooldownQuery);
	if (TimeRemainingAndDuration.IsEmpty())
	{
		return false;
	}

	// 如果同一组冷却身份下同时命中了多个效果，取剩余时间最长的那个为准。
	for (const TPair<float, float>& CooldownPair : TimeRemainingAndDuration)
	{
		if (CooldownPair.Key <= OutTimeRemaining)
		{
			continue;
		}

		OutTimeRemaining = CooldownPair.Key;
		OutTotalDuration = CooldownPair.Value;
	}

	return OutTimeRemaining > 0.0f;
}

void UAOSkillComponent::NotifySkillObservationChanged()
{
	OnSkillObservationChanged.Broadcast();
}

bool UAOSkillComponent::BuildSkillSlotViewData(const FAOSkillSlotEntry& SlotEntry, FAOSkillSlotViewData& OutViewData) const
{
	OutViewData = FAOSkillSlotViewData();
	OutViewData.SlotIndex = SlotEntry.SlotIndex;
	OutViewData.InputTag = SlotEntry.InputTag;
	OutViewData.SkillInstance = SlotEntry.SkillInstance;

	if (!SlotEntry.SkillInstance)
	{
		return true;
	}

	const UAOSkillInstance* SkillInstance = SlotEntry.SkillInstance;
	OutViewData.bHasSkill = true;
	OutViewData.SkillDefinition = SkillInstance->GetSkillDefinition();
	OutViewData.SourceItemInstance = SkillInstance->GetSourceItemInstance();
	OutViewData.SkillLevel = SkillInstance->GetSkillLevel();
	OutViewData.SkillQuality = SkillInstance->GetSkillQuality();

	ResolveSkillDisplayData(SkillInstance, OutViewData.SkillName, OutViewData.SkillDescription, OutViewData.SkillIcon, OutViewData.SourceItemDisplayName);
	OutViewData.bOnCooldown = GetSkillInstanceCooldownState(SkillInstance, OutViewData.CooldownRemaining, OutViewData.CooldownTotalDuration);
	return true;
}

bool UAOSkillComponent::BuildEquippedSkillViewData(const UAOSkillInstance* SkillInstance, FAOEquippedSkillViewData& OutViewData) const
{
	if (!SkillInstance)
	{
		return false;
	}

	OutViewData = FAOEquippedSkillViewData();
	OutViewData.SkillInstance = const_cast<UAOSkillInstance*>(SkillInstance);
	OutViewData.SkillDefinition = SkillInstance->GetSkillDefinition();
	OutViewData.SourceItemInstance = SkillInstance->GetSourceItemInstance();
	OutViewData.SlotIndex = SkillInstance->GetCurrentSlotIndex();
	OutViewData.SkillLevel = SkillInstance->GetSkillLevel();
	OutViewData.SkillQuality = SkillInstance->GetSkillQuality();

	ResolveSkillDisplayData(SkillInstance, OutViewData.SkillName, OutViewData.SkillDescription, OutViewData.SkillIcon, OutViewData.SourceItemDisplayName);
	OutViewData.bOnCooldown = GetSkillInstanceCooldownState(SkillInstance, OutViewData.CooldownRemaining, OutViewData.CooldownTotalDuration);
	return true;
}

void UAOSkillComponent::ResolveSkillDisplayData(const UAOSkillInstance* SkillInstance, FText& OutSkillName, FText& OutSkillDescription, TObjectPtr<UTexture2D>& OutSkillIcon, FText& OutSourceItemDisplayName) const
{
	OutSkillName = FText::GetEmpty();
	OutSkillDescription = FText::GetEmpty();
	OutSkillIcon = nullptr;
	OutSourceItemDisplayName = FText::GetEmpty();

	if (!SkillInstance)
	{
		return;
	}

	// 展示信息优先来自 SkillDefinition，因为那才是“技能是什么”。
	if (const UAOSkillDefinition* SkillDefinition = SkillInstance->GetSkillDefinition())
	{
		OutSkillName = SkillDefinition->SkillName;
		OutSkillDescription = SkillDefinition->SkillDescription;
		OutSkillIcon = SkillDefinition->SkillIcon;
	}

	if (const UAOInventoryItemInstance* SourceItemInstance = SkillInstance->GetSourceItemInstance())
	{
		if (const UAOInventoryItemDefinition* ItemDefinition = SourceItemInstance->GetItemCDO())
		{
			OutSourceItemDisplayName = FText::FromName(ItemDefinition->DisplayName);

			if (OutSkillName.IsEmpty())
			{
				OutSkillName = OutSourceItemDisplayName;
			}
		}
	}
}

bool UAOSkillComponent::GrantSkillToSlot(FAOSkillSlotEntry& SlotEntry)
{
	if (!SlotEntry.SkillInstance)
	{
		return false;
	}

	UAOAbilitySystem* AbilitySystem = GetAbilitySystemComponent();
	if (!AbilitySystem || !AbilitySystem->IsOwnerActorAuthoritative())
	{
		return false;
	}

	const UAOSkillDefinition* SkillDefinition = SlotEntry.SkillInstance->GetSkillDefinition();
	if (!SkillDefinition || !SkillDefinition->AbilityClass)
	{
		return false;
	}

	// 按当前槽位结果构建一个最小 RuntimeAbilitySet，复用现有授予/回收机制。
	UAOAbilitySet* RuntimeAbilitySet = NewObject<UAOAbilitySet>(this);
	FAOAbilitySet_GameplayAbility GrantedAbility;
	GrantedAbility.Ability = SkillDefinition->AbilityClass;
	GrantedAbility.AbilityLevel = FMath::Max(1, SlotEntry.SkillInstance->GetSkillLevel());
	GrantedAbility.InputTag = SlotEntry.InputTag;
	RuntimeAbilitySet->GrantedGameplayAbilities.Add(GrantedAbility);

	RuntimeAbilitySet->GiveToAbilitySystem(AbilitySystem, &SlotEntry.GrantedHandles, SlotEntry.SkillInstance);

	if (!SlotEntry.GrantedAbilitySpecHandle.IsValid())
	{
		SlotEntry.GrantedAbilitySpecHandle = SlotEntry.GrantedHandles.GetPrimaryAbilitySpecHandle();
	}

	return SlotEntry.GrantedAbilitySpecHandle.IsValid();
}

void UAOSkillComponent::RevokeSkillFromSlot(FAOSkillSlotEntry& SlotEntry)
{
	UAOAbilitySystem* AbilitySystem = GetAbilitySystemComponent();
	if (!AbilitySystem || !AbilitySystem->IsOwnerActorAuthoritative())
	{
		SlotEntry.GrantedAbilitySpecHandle = FGameplayAbilitySpecHandle();
		SlotEntry.GrantedHandles = FAOAbilitySet_GrantedHandles();
		return;
	}

	SlotEntry.GrantedHandles.TakeFromAbilitySystem(AbilitySystem);
	SlotEntry.GrantedAbilitySpecHandle = FGameplayAbilitySpecHandle();
}

bool UAOSkillComponent::RefreshGrantedAbilityForSlot(int32 SlotIndex)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !IsValidSkillSlotIndex(SlotIndex))
	{
		return false;
	}

	FAOSkillSlotEntry& SlotEntry = SkillSlots[SlotIndex];
	RevokeSkillFromSlot(SlotEntry);

	if (!SlotEntry.SkillInstance)
	{
		return true;
	}

	return GrantSkillToSlot(SlotEntry);
}
