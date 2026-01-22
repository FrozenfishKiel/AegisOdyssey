// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "AOQuickBarComponent.generated.h"

/**
 * 
 */
//rpc必然是可复制的，可复制的加const是什么鬼不能赋值，还怎么同步

UCLASS(BlueprintType)
class AEGISODYSSEY_API UAOQuickBarComponent : public UAOInventoryComponent , public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
public:
	UAOQuickBarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	UFUNCTION(Server,Reliable,BlueprintCallable)
	void SetActivateIndex(int32 NewIndex);
	virtual UAOInventoryComponent* GetInventoryComponent() override { return this; }
	virtual void BroadCastInventoryChange() override;

	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName;}
public:
	virtual void Client_BroadCastInventoryAdd(const TArrayView<int32> AddIndices, int32 FinalSize) override;
	virtual void Client_BroadCastInventoryChange(const TArrayView<int32> ChangedIndices, int32 FinalSize) override;
	virtual void Client_BroadCastInventoryRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize) override;
protected:
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void CheckDefaultInitialization() override;
	virtual void OnRegister() override;
	virtual void InitializeParams() override;
	virtual void InitializeOrRefreshInventorySlots() override;
	UFUNCTION()
	void OnRep_ActivateSlotIndex();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
private:
	UPROPERTY(ReplicatedUsing = OnRep_ActivateSlotIndex)
	int32 ActivateSlotIndex = -1;
private:
	void UseItemInSlot();
	void UnUseItemInSlot();

};

