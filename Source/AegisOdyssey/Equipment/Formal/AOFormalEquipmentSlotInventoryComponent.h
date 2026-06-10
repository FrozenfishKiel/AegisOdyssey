#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "AOFormalEquipmentSlotInventoryComponent.generated.h"

class UAOFormalEquipmentManagerComponent;
class UMVVM_InventoryMenu;

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOFormalEquipmentSlotInventoryComponent : public UAOInventoryComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	// 正式装备槽库存投影组件。
	//
	// 它不是正式装备运行时真相本身，而是把“正式装备栏当前五个槽里放了什么”
	// 投影成标准库存结构，好让统一库存交换链和现有 UI 直接复用。
	//
	// 真正的装备属性施加、槽位运行时状态维护，仍然交给 UAOFormalEquipmentManagerComponent。
	static const FName NAME_ActorFeatureName;

	UAOFormalEquipmentSlotInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual UAOInventoryComponent* GetInventoryComponent() override { return this; }

	UFUNCTION(BlueprintPure, Category = "FormalEquipment")
	UAOFormalEquipmentManagerComponent* GetOwningFormalEquipmentManager() const;

	// 正式装备栏给 UI / UMG 暴露的库存型 ViewModel。
	// 这里继续复用现有 InventoryMenu ViewModel，而不是立刻新起一套正式装备专用 MVVM 类型。
	UFUNCTION(BlueprintPure, Category = "FormalEquipment|UI")
	UMVVM_InventoryMenu* GetFormalEquipmentViewModel() const { return FormalEquipmentViewModel; }

	void SyncFormalEquipmentRuntimeFromInventoryProjection();

	virtual void BroadCastInventoryChange(int32 ChangedIndex = 0) override;
	virtual bool CanAcceptInventoryEntryAtSlot(const FAOInventoryEntry& IncomingEntry, int32 TargetSlotIndex) const override;
	virtual void BroadCastInventoryAddOnClient(const TArrayView<int32> AddIndices, int32 FinalSize, const TArray<FAOInventoryEntry>& TargetList) override;
	virtual void BroadCastInventoryChangeOnClient(const TArrayView<int32> ChangedIndices, int32 FinalSize) override;
	virtual void BroadCastInventoryRemoveOnClient(const TArrayView<int32> RemovedIndices, int32 FinalSize) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	virtual void InitializeParams() override;
	virtual void InitializeOrRefreshInventorySlots() override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void CheckDefaultInitialization() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_FormalEquipmentViewModel();

	// 单机或监听服本地玩家也要主动收到一次正式装备栏刷新广播。
	void NotifyLocalFormalEquipmentViewModelChanged() const;

private:
	UPROPERTY(ReplicatedUsing = OnRep_FormalEquipmentViewModel)
	TObjectPtr<UMVVM_InventoryMenu> FormalEquipmentViewModel = nullptr;
};
