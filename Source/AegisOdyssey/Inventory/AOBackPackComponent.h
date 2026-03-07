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
	virtual void BroadCastInventoryChange() override;

	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName;}
public:
	virtual void Client_BroadCastInventoryAdd(const TArrayView<int32> AddIndices, int32 FinalSize, const TArray<FAOInventoryEntry>& TargetList) override;
	virtual void Client_BroadCastInventoryChange(const TArrayView<int32> ChangedIndices, int32 FinalSize) override;
	virtual void Client_BroadCastInventoryRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize) override;
protected:
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnRegister() override;
	virtual void CheckDefaultInitialization() override;
	virtual void InitializeOrRefreshInventorySlots() override;
	virtual void InitializeParams() override;
	virtual void BeginPlay() override;
	
};
