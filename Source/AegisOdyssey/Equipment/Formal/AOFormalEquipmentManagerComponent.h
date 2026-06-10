#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySet.h"
#include "AegisOdyssey/Inventory/AOInventoryManagerComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentTypes.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "AOFormalEquipmentManagerComponent.generated.h"

class UAOFormalEquipmentInstance;
class UAOFormalEquipmentSlotInventoryComponent;
class UAOAbilitySystem;

USTRUCT(BlueprintType)
struct FAOFormalEquipmentSlotEntry
{
	GENERATED_BODY()

public:
	FAOFormalEquipmentSlotEntry() = default;

	FAOFormalEquipmentSlotEntry(const EAOFormalEquipmentSlotType InSlotType, const int32 InSlotIndex)
		: SlotType(InSlotType), SlotIndex(InSlotIndex)
	{
	}

	UPROPERTY(BlueprintReadOnly, Category = "FormalEquipment")
	EAOFormalEquipmentSlotType SlotType = EAOFormalEquipmentSlotType::None;

	UPROPERTY(BlueprintReadOnly, Category = "FormalEquipment")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "FormalEquipment")
	TObjectPtr<UAOFormalEquipmentInstance> EquippedInstance = nullptr;

	UPROPERTY()
	FAOAbilitySet_GrantedHandles GrantedHandles;

	void ResetRuntimeState()
	{
		EquippedInstance = nullptr;
		GrantedHandles = FAOAbilitySet_GrantedHandles();
	}
};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOFormalEquipmentManagerComponent : public UAOInventoryManagerComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	// 正式装备运行时真相组件。
	//
	// 它负责三件事：
	// 1. 维护五个正式装备槽当前各自装备了哪件正式装备实例；
	// 2. 在装备投影变化时，为对应槽位授予 / 回收 Definition.AbilitySetsToGrant；
	// 3. 提供“某件库存物品能不能进入某个正式槽”的统一运行时判断。
	//
	// 它不负责 UI 显示，也不负责武器激活态管理。
	static const FName NAME_ActorFeatureName;

	UAOFormalEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }

	static UAOFormalEquipmentManagerComponent* FindFormalEquipmentManagerComponent(const AActor* Actor)
	{
		return Actor ? Actor->FindComponentByClass<UAOFormalEquipmentManagerComponent>() : nullptr;
	}

	virtual void OnItemUse(FAOInventoryEntry& TargetItem) override;

	UFUNCTION(BlueprintPure, Category = "FormalEquipment")
	bool IsValidFormalSlotIndex(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "FormalEquipment")
	EAOFormalEquipmentSlotType GetFormalSlotTypeByIndex(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "FormalEquipment")
	bool CanAcceptInventoryItemForFormalSlot(const UAOInventoryItemInstance* InventoryItemInstance, int32 SlotIndex) const;

	// 对外的正式装备请求入口。
	// 客户端可以调用它，组件内部会在本地校验后决定直接执行还是走服务端 RPC。
	bool RequestEquipInventoryItemToSlot(UAOInventoryItemInstance* InventoryItemInstance, int32 SlotIndex);

	// 真正执行一次“把库存实例装备进正式槽”。
	// 这里最终仍然复用统一库存交换主链，而不是自己手写一套装备迁移。
	bool EquipInventoryItemToSlot(UAOInventoryItemInstance* InventoryItemInstance, int32 SlotIndex);

	// 对外的正式装备卸下请求入口。
	// 这条链主要服务正式装备栏自己的右键菜单等表现层交互。
	bool RequestUnequipFormalSlot(int32 SlotIndex);
	bool UnequipFormalSlot(int32 SlotIndex);

	// 把正式装备槽库存投影重新翻译回运行时真相。
	// 只要投影槽位内容发生变化，这里就负责同步 EquippedInstance 与对应的 AbilitySet 授予句柄。
	void SyncFormalEquipmentFromInventoryProjection(const TArray<FAOInventoryEntry>& Slots);

	const TArray<FAOFormalEquipmentSlotEntry>& GetFormalEquipmentSlots() const { return FormalEquipmentSlots; }

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void CheckDefaultInitialization() override;

	UFUNCTION()
	void OnRep_FormalEquipmentSlots();

	UFUNCTION(Server, Reliable)
	void ServerRequestEquipInventoryItemToSlot(UAOInventoryItemInstance* InventoryItemInstance, int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerRequestUnequipFormalSlot(int32 SlotIndex);

private:
	void InitializeFormalSlots();
	UAOAbilitySystem* GetAbilitySystemComponent() const;
	bool ApplyAbilitySetsForSlot(FAOFormalEquipmentSlotEntry& SlotEntry);
	void RemoveAbilitySetsForSlot(FAOFormalEquipmentSlotEntry& SlotEntry);
	const UAOFormalEquipmentInstance* ResolveFormalEquipmentInstance(const UAOInventoryItemInstance* InventoryItemInstance) const;
	UAOInventoryComponent* FindOwningInventoryContainingItem(const UAOInventoryItemInstance* InventoryItemInstance) const;

	UPROPERTY(ReplicatedUsing = OnRep_FormalEquipmentSlots)
	TArray<FAOFormalEquipmentSlotEntry> FormalEquipmentSlots;
};
