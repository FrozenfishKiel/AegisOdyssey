// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "AOQuickBarComponent.generated.h"

class UMVVM_InventoryMenu;

UCLASS(BlueprintType)
class AEGISODYSSEY_API UAOQuickBarComponent : public UAOInventoryComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UAOQuickBarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetActivateIndex(int32 NewIndex);

	virtual UAOInventoryComponent* GetInventoryComponent() override { return this; }
	virtual void BroadCastInventoryChange(int32 ChangedIndex) override;

	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }

	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	void CycleActiveSlotForward();

	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	void CycleActiveSlotBackward();

public:
	virtual void BroadCastInventoryAddOnClient(const TArrayView<int32> AddIndices, int32 FinalSize, const TArray<FAOInventoryEntry>& TargetList) override;
	virtual void BroadCastInventoryChangeOnClient(const TArrayView<int32> ChangedIndices, int32 FinalSize) override;
	virtual void BroadCastInventoryRemoveOnClient(const TArrayView<int32> RemovedIndices, int32 FinalSize) override;
	UMVVM_InventoryMenu* GetQuickBarViewModel() const { return QuickBarViewModel; }

protected:
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void CheckDefaultInitialization() override;
	virtual void OnRegister() override;
	virtual void InitializeParams() override;
	virtual void InitializeOrRefreshInventorySlots() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_ActivateSlotIndex(int32 LastActivateSlotIndex);

	UFUNCTION()
	void OnRep_QuickBarViewModel();

	// 单机或监听服本地玩家也需要主动收到一次 ViewModel 刷新广播。
	void NotifyLocalQuickBarViewModelChanged() const;

private:
	void UseItemInSlot(int32 OldIndex, int32 NewIndex);
	void UnUseItemInSlot(int32 NewIndex, int32 OldIndex);

private:
	UPROPERTY(ReplicatedUsing = OnRep_ActivateSlotIndex)
	int32 ActivateSlotIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_QuickBarViewModel)
	TObjectPtr<UMVVM_InventoryMenu> QuickBarViewModel = nullptr;
};
