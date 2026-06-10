// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "AOBackPackComponent.generated.h"

/**
 * 
 */
class UMVVM_InventoryMenu;

UCLASS()
class AEGISODYSSEY_API UAOBackPackComponent : public UAOInventoryComponent ,public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
public:
	UAOBackPackComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BroadCastInventoryChange(int32 ChangedIndex) override;

	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName;}
public:
	virtual void BroadCastInventoryAddOnClient(const TArrayView<int32> AddIndices, int32 FinalSize, const TArray<FAOInventoryEntry>& TargetList) override;
	virtual void BroadCastInventoryChangeOnClient(const TArrayView<int32> ChangedIndices, int32 FinalSize) override;
	virtual void BroadCastInventoryRemoveOnClient(const TArrayView<int32> RemovedIndices, int32 FinalSize) override;
	UMVVM_InventoryMenu* GetInventoryViewModel() const {return BackPackViewModel;}
protected:
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnRegister() override;
	virtual void CheckDefaultInitialization() override;
	virtual void InitializeOrRefreshInventorySlots() override;
	virtual void InitializeParams() override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	UFUNCTION()
	void OnRep_BackPackViewModel();

	// 单机或监听服务器本地玩家也需要主动收到一次 ViewModel 刷新广播。
	void NotifyLocalInventoryViewModelChanged() const;
private:
	UPROPERTY(ReplicatedUsing = OnRep_BackPackViewModel)
	TObjectPtr<UMVVM_InventoryMenu> BackPackViewModel;
};
