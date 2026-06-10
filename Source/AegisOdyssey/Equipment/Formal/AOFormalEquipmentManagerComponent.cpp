#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.h"

#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySet.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentDefinition.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentInstance.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentSlotInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOBackPackComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOFormalEquipmentManagerComponent)

const FName UAOFormalEquipmentManagerComponent::NAME_ActorFeatureName("FormalEquipment");

UAOFormalEquipmentManagerComponent::UAOFormalEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UAOFormalEquipmentManagerComponent::OnItemUse(FAOInventoryEntry& TargetItem)
{
	// 正式装备仍然复用“从库存里使用物品”的入口。
	// 这里不直接写死头盔槽 / 鞋子槽，而是先从定义里解析正式槽类型，
	// 再把这件来源实例路由到它对应的唯一正式槽。
	if (TargetItem.Instance == nullptr)
	{
		return;
	}
	UAOFormalEquipmentInstance* FormalEquipmentInstance = Cast<UAOFormalEquipmentInstance>(TargetItem.Instance);
	if (FormalEquipmentInstance == nullptr)
	{
		return;
	}

	const UAOFormalEquipmentDefinition* FormalEquipmentDefinition = FormalEquipmentInstance->GetFormalEquipmentDefinition();
	if (FormalEquipmentDefinition == nullptr)
	{
		return;
	}

	const EAOFormalEquipmentSlotType SlotType = FormalEquipmentDefinition->GetFormalSlotType();
	for (int32 SlotIndex = 0; SlotIndex < FormalEquipmentSlots.Num(); ++SlotIndex)
	{
		if (FormalEquipmentSlots[SlotIndex].SlotType == SlotType)
		{
			RequestEquipInventoryItemToSlot(TargetItem.Instance, SlotIndex);
			return;
		}
	}
}

bool UAOFormalEquipmentManagerComponent::IsValidFormalSlotIndex(int32 SlotIndex) const
{
	return FormalEquipmentSlots.IsValidIndex(SlotIndex);
}

EAOFormalEquipmentSlotType UAOFormalEquipmentManagerComponent::GetFormalSlotTypeByIndex(int32 SlotIndex) const
{
	if (!IsValidFormalSlotIndex(SlotIndex))
	{
		return EAOFormalEquipmentSlotType::None;
	}

	return FormalEquipmentSlots[SlotIndex].SlotType;
}

bool UAOFormalEquipmentManagerComponent::CanAcceptInventoryItemForFormalSlot(const UAOInventoryItemInstance* InventoryItemInstance, int32 SlotIndex) const
{
	if (!IsValidFormalSlotIndex(SlotIndex))
	{
		return false;
	}

	// 这里只认“正式装备实例”，从根上挡掉武器、消耗品、材料等非正式装备来源物。
	const UAOFormalEquipmentInstance* FormalEquipmentInstance = ResolveFormalEquipmentInstance(InventoryItemInstance);
	if (FormalEquipmentInstance == nullptr)
	{
		return false;
	}

	const UAOFormalEquipmentDefinition* FormalEquipmentDefinition = FormalEquipmentInstance->GetFormalEquipmentDefinition();
	if (FormalEquipmentDefinition == nullptr)
	{
		return false;
	}

	return FormalEquipmentDefinition->GetFormalSlotType() == FormalEquipmentSlots[SlotIndex].SlotType;
}

bool UAOFormalEquipmentManagerComponent::RequestEquipInventoryItemToSlot(UAOInventoryItemInstance* InventoryItemInstance, int32 SlotIndex)
{
	if (!CanAcceptInventoryItemForFormalSlot(InventoryItemInstance, SlotIndex))
	{
		return false;
	}

	// 服务端持权时直接执行；
	// 客户端只负责把已经通过本地基础校验的请求转发给服务端。
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		return EquipInventoryItemToSlot(InventoryItemInstance, SlotIndex);
	}

	ServerRequestEquipInventoryItemToSlot(InventoryItemInstance, SlotIndex);
	return true;
}

bool UAOFormalEquipmentManagerComponent::EquipInventoryItemToSlot(UAOInventoryItemInstance* InventoryItemInstance, int32 SlotIndex)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !CanAcceptInventoryItemForFormalSlot(InventoryItemInstance, SlotIndex))
	{
		return false;
	}

	// 正式装备的来源必须是“库存里当前真实存在的那件实例”。
	// 所以这里先反查它现在到底属于哪个库存容器、哪个槽位，
	// 再统一交给库存交换主链去处理迁移与替换。
	UAOInventoryComponent* OwningInventory = FindOwningInventoryContainingItem(InventoryItemInstance);
	UAOFormalEquipmentSlotInventoryComponent* FormalInventory = GetOwner()->FindComponentByClass<UAOFormalEquipmentSlotInventoryComponent>();
	if (OwningInventory == nullptr || FormalInventory == nullptr)
	{
		return false;
	}

	const int32 InventorySlotIndex = OwningInventory->FindInventorySlotIndexFromInstance(InventoryItemInstance);
	if (InventorySlotIndex == INDEX_NONE)
	{
		return false;
	}

	return UAOInventoryComponent::ExecuteExchangeRequest(OwningInventory, InventorySlotIndex, FormalInventory, SlotIndex);
}

bool UAOFormalEquipmentManagerComponent::RequestUnequipFormalSlot(int32 SlotIndex)
{
	if (!IsValidFormalSlotIndex(SlotIndex))
	{
		return false;
	}

	if (GetOwner() != nullptr && GetOwner()->HasAuthority())
	{
		return UnequipFormalSlot(SlotIndex);
	}

	ServerRequestUnequipFormalSlot(SlotIndex);
	return true;
}

bool UAOFormalEquipmentManagerComponent::UnequipFormalSlot(int32 SlotIndex)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !IsValidFormalSlotIndex(SlotIndex))
	{
		return false;
	}

	FAOFormalEquipmentSlotEntry& SlotEntry = FormalEquipmentSlots[SlotIndex];
	if (SlotEntry.EquippedInstance == nullptr)
	{
		return false;
	}

	UAOFormalEquipmentSlotInventoryComponent* FormalInventory = GetOwner()->FindComponentByClass<UAOFormalEquipmentSlotInventoryComponent>();
	UAOBackPackComponent* BackPackComponent = GetOwner()->FindComponentByClass<UAOBackPackComponent>();
	if (FormalInventory == nullptr || BackPackComponent == nullptr)
	{
		return false;
	}

	// 正式装备槽右键卸下的首版规则很明确：
	// 不是回发到某个历史来源槽，而是回到角色“库存”的主接收容器。
	const int32 TargetInventorySlotIndex = BackPackComponent->FindAvaliableSlot(SlotEntry.EquippedInstance, 1);
	if (TargetInventorySlotIndex == INDEX_NONE)
	{
		return false;
	}

	return UAOInventoryComponent::ExecuteExchangeRequest(FormalInventory, SlotIndex, BackPackComponent, TargetInventorySlotIndex);
}

void UAOFormalEquipmentManagerComponent::SyncFormalEquipmentFromInventoryProjection(const TArray<FAOInventoryEntry>& Slots)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || Slots.Num() < FormalEquipmentSlots.Num())
	{
		return;
	}

	// 正式装备栏的运行时真相不直接信任 UI，也不自己维护另一份独立容器。
	// 它始终以 FormalEquipmentSlotInventoryComponent 当前的库存投影为输入，
	// 再把每个槽翻译回 EquippedInstance + 已授予 AbilitySet 句柄。
	for (int32 SlotIndex = 0; SlotIndex < FormalEquipmentSlots.Num(); ++SlotIndex)
	{
		FAOFormalEquipmentSlotEntry& SlotEntry = FormalEquipmentSlots[SlotIndex];
		UAOFormalEquipmentInstance* IncomingInstance = Cast<UAOFormalEquipmentInstance>(Slots[SlotIndex].Instance);

		// 这里再次做运行时限类校验，避免外部有人绕过 UI 把错误物品塞进正式槽投影。
		if (IncomingInstance != nullptr && !CanAcceptInventoryItemForFormalSlot(IncomingInstance, SlotIndex))
		{
			continue;
		}

		// 同一个槽当前装备实例没有变化时，不要重复回收 / 重授 AbilitySet，
		// 否则后面很容易把句柄链路和叠加结果弄脏。
		if (SlotEntry.EquippedInstance == IncomingInstance)
		{
			continue;
		}

		RemoveAbilitySetsForSlot(SlotEntry);
		SlotEntry.EquippedInstance = IncomingInstance;
		ApplyAbilitySetsForSlot(SlotEntry);
	}

	OnRep_FormalEquipmentSlots();
}

void UAOFormalEquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UAOFormalEquipmentManagerComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();
}

void UAOFormalEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, FormalEquipmentSlots);
}

void UAOFormalEquipmentManagerComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	IGameFrameworkInitStateInterface::OnActorInitStateChanged(Params);
}

void UAOFormalEquipmentManagerComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == AOGameplayTags::InitState_DataInitialized && DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		// 正式槽是固定五槽，所以初始化时一次性把槽位骨架建好。
		InitializeFormalSlots();
	}
}

void UAOFormalEquipmentManagerComponent::CheckDefaultInitialization()
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

void UAOFormalEquipmentManagerComponent::OnRep_FormalEquipmentSlots()
{
	// 当前先保留为空。
	// 如果后续要做正式装备栏 UI 的显式客户端刷新桥接，可以从这里补发通知。
}

void UAOFormalEquipmentManagerComponent::ServerRequestEquipInventoryItemToSlot_Implementation(UAOInventoryItemInstance* InventoryItemInstance, int32 SlotIndex)
{
	EquipInventoryItemToSlot(InventoryItemInstance, SlotIndex);
}

void UAOFormalEquipmentManagerComponent::ServerRequestUnequipFormalSlot_Implementation(int32 SlotIndex)
{
	UnequipFormalSlot(SlotIndex);
}

void UAOFormalEquipmentManagerComponent::InitializeFormalSlots()
{
	if (!FormalEquipmentSlots.IsEmpty())
	{
		return;
	}

	// 正式装备栏这一轮明确锁成五个唯一槽。
	// 索引顺序后续也会成为 UI 建槽和库存投影对齐的基础，不要随意调整。
	FormalEquipmentSlots.Reserve(5);
	FormalEquipmentSlots.Add(FAOFormalEquipmentSlotEntry(EAOFormalEquipmentSlotType::Helmet, 0));
	FormalEquipmentSlots.Add(FAOFormalEquipmentSlotEntry(EAOFormalEquipmentSlotType::Armor, 1));
	FormalEquipmentSlots.Add(FAOFormalEquipmentSlotEntry(EAOFormalEquipmentSlotType::Gloves, 2));
	FormalEquipmentSlots.Add(FAOFormalEquipmentSlotEntry(EAOFormalEquipmentSlotType::Necklace, 3));
	FormalEquipmentSlots.Add(FAOFormalEquipmentSlotEntry(EAOFormalEquipmentSlotType::Boots, 4));
}

UAOAbilitySystem* UAOFormalEquipmentManagerComponent::GetAbilitySystemComponent() const
{
	const APawn* OwnerPawn = GetPawn<APawn>();
	if (OwnerPawn == nullptr)
	{
		return nullptr;
	}

	if (const UAOExtPawnComponent* ExtPawnComponent = UAOExtPawnComponent::FindAOExtPawnComponent(OwnerPawn))
	{
		return ExtPawnComponent->GetAOAbilitySystemComponent();
	}

	return nullptr;
}

bool UAOFormalEquipmentManagerComponent::ApplyAbilitySetsForSlot(FAOFormalEquipmentSlotEntry& SlotEntry)
{
	// 先清空旧句柄，确保这一轮记录到的只会是“当前这件装备”授予出来的结果。
	// 这样后面卸下或替换时，才能按件回收，而不是模糊地整组清理。
	SlotEntry.GrantedHandles = FAOAbilitySet_GrantedHandles();

	const UAOFormalEquipmentInstance* EquippedInstance = SlotEntry.EquippedInstance;
	const UAOFormalEquipmentDefinition* FormalEquipmentDefinition = EquippedInstance ? EquippedInstance->GetFormalEquipmentDefinition() : nullptr;
	UAOAbilitySystem* AbilitySystem = GetAbilitySystemComponent();
	if (FormalEquipmentDefinition == nullptr || AbilitySystem == nullptr)
	{
		return false;
	}

	bool bGrantedAnyAbilitySet = false;
	for (const TObjectPtr<UAOAbilitySet>& AbilitySet : FormalEquipmentDefinition->GetAbilitySetsToGrant())
	{
		if (AbilitySet == nullptr)
		{
			continue;
		}

		// 正式装备现在和武器统一走 Definition.AbilitySetsToGrant。
		// 这样属性 GE、能力和附带 AttributeSet 都复用同一套授予 / 回收句柄机制。
		AbilitySet->GiveToAbilitySystem(AbilitySystem, &SlotEntry.GrantedHandles, SlotEntry.EquippedInstance);
		bGrantedAnyAbilitySet = true;
	}

	return bGrantedAnyAbilitySet;
}

void UAOFormalEquipmentManagerComponent::RemoveAbilitySetsForSlot(FAOFormalEquipmentSlotEntry& SlotEntry)
{
	UAOAbilitySystem* AbilitySystem = GetAbilitySystemComponent();
	if (AbilitySystem == nullptr)
	{
		SlotEntry.GrantedHandles = FAOAbilitySet_GrantedHandles();
		return;
	}

	// 正式装备统一走 stack-aware 回收。
	// 这样不管是右键卸下、替换旧装备，还是别的正式装备移除语义，
	// 只要最终进入“正式装备槽正在回收这件装备授予结果”这一步，就会对可堆叠 GE 按层处理。
	SlotEntry.GrantedHandles.TakeFromAbilitySystemStackAware(AbilitySystem);
}

const UAOFormalEquipmentInstance* UAOFormalEquipmentManagerComponent::ResolveFormalEquipmentInstance(const UAOInventoryItemInstance* InventoryItemInstance) const
{
	return Cast<UAOFormalEquipmentInstance>(InventoryItemInstance);
}

UAOInventoryComponent* UAOFormalEquipmentManagerComponent::FindOwningInventoryContainingItem(const UAOInventoryItemInstance* InventoryItemInstance) const
{
	if (InventoryItemInstance == nullptr || GetOwner() == nullptr)
	{
		return nullptr;
	}

	// 不再使用外层对象推断来源容器，而是显式扫描角色身上的库存组件。
	// 这样来源物无论当前在背包、快捷栏还是别的库存投影里，只要它真属于该角色，
	// 都能被定位到当前真实容器。
	TArray<UAOInventoryComponent*> InventoryComponents;
	GetOwner()->GetComponents(InventoryComponents);

	for (UAOInventoryComponent* InventoryComponent : InventoryComponents)
	{
		if (InventoryComponent != nullptr && InventoryComponent->FindInventorySlotIndexFromInstance(InventoryItemInstance) != INDEX_NONE)
		{
			return InventoryComponent;
		}
	}

	return nullptr;
}
