// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"
#include "AOEquipmentInstance.generated.h"

struct FAOEquipmentSpawnedConfig;
/**
 * 
 */
class UAOInventoryItemDefinition;
UCLASS(BlueprintType, Blueprintable , DefaultToInstanced)
class AEGISODYSSEY_API UAOEquipmentInstance : public UAOInventoryItemInstance
{
	GENERATED_BODY()
public:
	UAOEquipmentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UWorld* GetWorld() const override final;  //禁止派生类继承这个函数

	UFUNCTION(BlueprintPure , Category = Equipment)
	UObject* GetInstigator() const {return Instigator;}
	
	void SetInstigator(UObject* InInstigator) {Instigator = InInstigator;}

	UFUNCTION(BlueprintPure , Category = Equipment)
	APawn* GetPawn() const;

	UFUNCTION(BlueprintPure , Category = Equipment , meta = (DeterminesOutputType = PawnType))
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;

	UFUNCTION(BlueprintPure , Category = Equipment)
	TArray<AActor*> GetSpawnedActors() const {return SpawnedActors;}
	
	virtual void SpawnEquipmentActors(const TArray<FAOEquipmentSpawnedConfig> SpawnConfigList);
	virtual void DestoryEquipmentActors();

	virtual void OnEquiped();
	virtual void OnUnEquiped();

	virtual UAOInventoryManagerComponent* FindTargetInventoryManager()  const override;

private:
	UFUNCTION()
	void OnRep_Instigator();
protected:
	virtual void SetItemDef(TSubclassOf<UAOInventoryItemDefinition> InDef) override;
protected:
	UPROPERTY(ReplicatedUsing = OnRep_Instigator)
	TObjectPtr<UObject> Instigator;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<AActor>> SpawnedActors;  //临时记录生成的Actors

protected:
	UFUNCTION(BlueprintImplementableEvent , Category = Equipment , meta = (DisplayName = "OnEquipped"))
	void K2_OnEquipped();

	UFUNCTION(BlueprintImplementableEvent , Category = Equipment , meta = (DisplayName = "OnUnEquipped"))
	void K2_OnUnEquipped();
};

